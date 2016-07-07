/**
 *  ReyPIC – Simulation Class Source
 * ==================================
 */

#include "clsSimulation.hpp"

using namespace std;
using namespace reypic;

// ****************************************************************************************************************** //

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

// ****************************************************************************************************************** //
//                                                Setters and Getters                                                 //
// ****************************************************************************************************************** //

/**
 *  Method :: SetRunMode
 * ======================
 *  Related to switch -t.
 *  In test mode the simulation is set up, but exits before the solver starts.
 */

bool Simulation::setRunMode(value_t iRunMode) {

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

// ****************************************************************************************************************** //
//                                                 Main Class Methods                                                 //
// ****************************************************************************************************************** //

/**
 *  Method :: Setup
 * =================
 */

error_t Simulation::Setup(Input_t* simInput) {

    error_t errVal = ERR_NONE;

    errVal = simInput->ReadVariable(INPUT_CONF, 0, "nodes", &m_Nodes, INVAR_INT);
    if(errVal != ERR_NONE) return errVal;

    errVal = simInput->ReadVariable(INPUT_CONF, 0, "threads", &m_Threads, INVAR_INT);
    if(errVal != ERR_NONE) return errVal;

    // Force m_Nodes to be equal to m_MPISize.
    // This makes m_Nodes redundant as an input variable, but the intention is to have m_Nodes be
    // able to split the grid in other dimensions than x1.
    m_Nodes = m_MPISize;

    if(m_Nodes < 1)   m_Nodes = 1;
    if(m_Threads < 1) m_Threads = 1;

    errVal = simInput->ReadVariable(INPUT_SIM, 0, "n0", &m_N0, INVAR_DOUBLE);
    if(errVal != ERR_NONE) return errVal;

    if(m_isMaster) {
        printf("  Configuration\n");
        printf(" ===============\n");
        printf("  Nodes:        %d\n", m_Nodes);
        printf("  Threads/node: %d\n", m_Threads);
        printf("\n");
    }

    return ERR_NONE;
}

// ****************************************************************************************************************** //

// End Class Simulation
