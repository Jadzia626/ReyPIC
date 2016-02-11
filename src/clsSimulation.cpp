/**
 *  ReyPIC – Simulation Class Source
 * ==================================
 *  This class wraps the entire simulation.
 *
 *  Sequence:
 *  1. Read input file.
 *  2. Setup simulation. This creates the grid and the particle arrays.
 *  3. Read restart information. Only if restart is specified. Exits if restart files do not match
 *     the setup from previous stage.
 *  4. Main simulation loop.
 */

#include "clsSimulation.hpp"

using namespace std;
using namespace reypic;

// ********************************************************************************************** //

/**
 *  Class Constructor
 * ===================
 */

Simulation::Simulation() {

    // Read MPI setup
    MPI_Comm_size(MPI_COMM_WORLD, &m_MPISize);
    MPI_Comm_rank(MPI_COMM_WORLD, &m_MPIRank);
    m_isMaster = (m_MPIRank == 0);

}

// ********************************************************************************************** //
//                                      Setters and Getters                                       //
// ********************************************************************************************** //

/**
 *  Set Input File
 * ================
 *  Related to the switch -i.
 *  Sets the InputFile variable.
 */

bool Simulation::setInputFile(char* cFile) {

    m_InputFile = cFile;

    return true;
}

// ********************************************************************************************** //

/**
 *  Set Run Mode
 * ==============
 *  Related to switch -t.
 *  In test mode the simulation is set up, but exits before the solver starts.
 */

bool Simulation::setRunMode(int iRunMode) {

    switch(iRunMode) {
        case RUN_MODE_FULL:
            m_RunMode = RUN_MODE_FULL;
            return true;
            break;
        case RUN_MODE_TEST:
            m_RunMode = RUN_MODE_TEST;
            return true;
            break;
        case RUN_MODE_EXT_TEST:
            m_RunMode = RUN_MODE_EXT_TEST;
            return true;
            break;
    }

    return false;
}

// ********************************************************************************************** //

/**
 *  isMaster
 * ==========
 *  Returns true if node is rank 0.
 */

bool Simulation::isMaster() {
    return m_isMaster;
}

// ********************************************************************************************** //
//                                       Main Class Methods                                       //
// ********************************************************************************************** //

/**
 *  Read Input File
 * =================
 *  Loads the input file
 */

int Simulation::ReadInput() {

    int errFile = ERR_NONE;

    // Read input file
    errFile = simInput.ReadFile(m_InputFile);
    if(errFile != ERR_NONE) {
        return errFile;
    }

    // Split input file sections
    errFile = simInput.SplitSections();
    if(errFile != ERR_NONE) {
        return errFile;
    }

    // Get number of species
    m_NumSpecies = simInput.getNumSpecies();

    return ERR_NONE;
}

// ********************************************************************************************** //

/**
 *  Setup
 * =======
 *  Sets up simulation
 */

int Simulation::Setup() {

    int nErr = 0;

    // Parallelisation
    nErr += simInput.ReadVariable(INPUT_SIM, 0, "nodes",   &m_Nodes,    INVAR_INT);
    nErr += simInput.ReadVariable(INPUT_SIM, 0, "threads", &m_Threads,  INVAR_INT);

    // Physics
    nErr += simInput.ReadVariable(INPUT_SIM, 0, "n0",      &m_N0,       INVAR_DOUBLE);

    // Time
    nErr += simInput.ReadVariable(INPUT_SIM, 0, "dt",      &m_TimeStep, INVAR_DOUBLE);
    nErr += simInput.ReadVariable(INPUT_SIM, 0, "tmin",    &m_TMin,     INVAR_DOUBLE);
    nErr += simInput.ReadVariable(INPUT_SIM, 0, "tmax",    &m_TMax,     INVAR_DOUBLE);

    // Force m_Nodes to be equal to m_MPISize.
    // This makes m_Nodes redundant as an input variable, but the intention is to have m_Nodes be
    // able to split the domain in other dimensions than x1.
    m_Nodes = m_MPISize;

    if(m_isMaster) {
        cout << "  Errors:   " << nErr << endl;
        cout << "  Nodes:    " << m_Nodes << endl;
        cout << "  Threads:  " << m_Threads << endl;
        cout << "  N0:       " << m_N0 << endl;
        cout << "  TimeStep: " << m_TimeStep << endl;
        cout << "  TMin:     " << m_TMin << endl;
        cout << "  TMax:     " << m_TMax << endl;
        cout << endl;
    }

    return ERR_NONE;
}

// ********************************************************************************************** //

// End Class Input
