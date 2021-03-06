/**
 * ReyPIC – Grid Source
 */

#include "clsGrid.hpp"

using namespace std;
using namespace reypic;

// ********************************************************************************************** //

/**
 *  Class Constructor
 * ===================
 */

Grid::Grid() {

    // Read MPI setup
    MPI_Comm_size(MPI_COMM_WORLD, &m_MPISize);
    MPI_Comm_rank(MPI_COMM_WORLD, &m_MPIRank);
    m_isMaster = (m_MPIRank == 0);

}

// ********************************************************************************************** //
//                                       Main Class Methods                                       //
// ********************************************************************************************** //

/**
 *  Setup
 * =======
 *  Sets up the grid
 */

error_t Grid::Setup(Input_t* simInput) {

    error_t errVal = ERR_NONE;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "ngrid", &m_NGrid, INVAR_VINT);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "xmin", &m_XMin, INVAR_VDOUBLE);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "xmax", &m_XMax, INVAR_VDOUBLE);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "gridres", &m_GridRes, INVAR_VSTRING);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "gridmin", &m_GridMin, INVAR_VDOUBLE);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "linpoint", &m_LinPoint, INVAR_VDOUBLE);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_GRID, 0, "gridfunc", &m_GridFunc, INVAR_VSTRING);
    if(errVal != ERR_NONE) return errVal;

    // Set up grid resolution vectors
    if(!setupGridDelta()) return ERR_SETUP;

    return ERR_NONE;
}

// ********************************************************************************************** //
//                                        Member Functions                                        //
// ********************************************************************************************** //

/**
 *  The setupGridDelta
 * ====================
 */

bool Grid::setupGridDelta() {

    vstring_t vsGridVars = {"n","N"};

    for(index_t iDim=0; iDim<3; iDim++) {

        vdouble_t vEval;

        double_t  linPoint = m_LinPoint[iDim];
        double_t  delMin   = m_GridMin[iDim];
        double_t  xMin     = m_XMin[iDim];
        double_t  xMax     = m_XMax[iDim];
        index_t   nGrid    = m_NGrid[iDim];
        double_t  xSpan    = xMax - xMin;
        double_t  delAvg   = xSpan/nGrid;

        // Check validity
        if(nGrid < 1) {
            if(m_isMaster) {
                printf("  Grid Error: Invalid ngrid entry %d (ngrid >= 1)\n", (int)nGrid);
            }
            return false;
        }
        if(xSpan <= 0.0) {
            if(m_isMaster) {
                printf("  Grid Error: Invalid xmin/xmax entry %.4f – %.4f (xmax-xmin > 0)\n", xMin, xMax);
            }
            return false;
        }

        if(m_GridRes[iDim] == "fixed") {

            // All cells are the same size
            vEval.assign(nGrid, delAvg);

            if(m_isMaster) {
                printf("  Grid resolution in x%d: %.4f\n", (int)iDim+1, delAvg);
            }

        } else
        if(m_GridRes[iDim] == "linear") {

            // Cell size decreases towards the specified grid index from enpoint(s)

            if(delMin <= 0.0) {
                if(m_isMaster) {
                    printf("  Grid Error: Invalid gridmin entry %.4f (gridmin > 0)\n", delMin);
                }
                return false;
            }
            if(linPoint < 0.0 || linPoint > 1.0*nGrid) {
                if(m_isMaster) {
                    printf("  Grid Error: Invalid linpoint entry %.1f (0 <= linpoint <= ngrid)\n", linPoint);
                }
                return false;
            }

            double_t aEval[nGrid] = {0.0};
            double_t valMin, valMax;
            index_t  linLower  = (index_t)floor(linPoint);
            index_t  linHigher = (index_t)ceil(linPoint);

            if(linLower == 0) {
                m::linspace(delMin, 2*delAvg-delMin, nGrid, aEval);
            } else
            if(linHigher == nGrid-1) {
                m::linspace(2*delAvg-delMin, delMin, nGrid, aEval);
            } else {
                m::linspace(2*delAvg-delMin, delMin, linHigher, aEval);
                m::linspace(delMin, 2*delAvg-delMin, nGrid-linHigher, aEval+linHigher);
            }

            valMin = m::min(aEval, nGrid);
            valMax = m::max(aEval, nGrid);

            if(m_isMaster) {
                printf("  Grid resolution in x%d: %.4f – %.4f\n", (int)iDim+1, valMin, valMax);
            }

        } else
        if(m_GridRes[iDim] == "func") {

            if(delMin <= 0.0) {
                if(m_isMaster) {
                    printf("  Grid Error: Invalid gridmin entry %.4f (gridmin > 0)\n", delMin);
                }
                return false;
            }

            Math_t mFunc;

            if(!mFunc.setVariables(vsGridVars))      return false;
            if(!mFunc.setEquation(m_GridFunc[iDim])) return false;

            // Evaluate function by normalisint it to the span of the grid (xMax - xMin)
            // including an offset determined by delMin

            vdouble_t vdEval = {0.0,1.0*nGrid};
            double_t  aEval[nGrid] = {0.0};
            double_t  dScale, valMin, valMax, valSum;

            for(index_t i=0; i<nGrid; i++) {
                vdEval[0] = i+0.5;
                if(!mFunc.Eval(vdEval,&aEval[i])) return false;
            }

            // Invert array so that highest value gives highest density, and offset to max value
            valMax = m::max(aEval, nGrid);
            m::scale(aEval, nGrid, -1);
            m::offset(aEval, nGrid, valMax);

            // Normalise to size of domain and offset by minimum grid size
            valSum = m::sum(aEval, nGrid);
            dScale = xSpan - nGrid*delMin;
            m::scale(aEval, nGrid, dScale/valSum);
            m::offset(aEval, nGrid, delMin);

            vEval.assign(aEval, aEval+nGrid);

            valMin = m::min(aEval, nGrid);
            valMax = m::max(aEval, nGrid);

            if(m_isMaster) {
                printf("  Grid resolution in x%d: %.4f – %.4f\n", (int)iDim+1, valMin, valMax);
            }
        }

        gridDelta.push_back(vEval);
    }

    return true;
}

// ********************************************************************************************** //

// End Class Grid
