/**
 * ReyPIC – Simulation Header
 */

#ifndef CLASS_SIMULATION
#define CLASS_SIMULATION

#include "config.hpp"

// Dependent Classes
#include "clsInput.hpp"

namespace reypic {

class Simulation {

public:

   /**
    * Constructor/Destructor
    */

    Simulation();
    ~Simulation() {};

   /**
    * Setters/Getters
    */

    bool setInputFile(char*);
    bool setRunMode(int);

    bool isMaster();

   /**
    * Methods
    */

    int  ReadInput();    // Read input file
    int  Setup();        //
    void ReadRestart();
    void MainLoop();
    int  AbortExec(int);
    int  Finalize(int);

   /**
    * Properties
    */

    Input simInput;

private:

   /**
    * Member Variables
    */

    // General
    char*  m_InputFile;
    int    m_RunMode    = RUN_MODE_FULL;
    int    m_NumSpecies = 0;

    // Parallelisation
    int    m_MPISize     =  0;              // Number of nodes
    int    m_MPIRank     = -1;              // Node number
    bool   m_isMaster    = false;           // True if this node is master

    int    m_Nodes      =  1;
    int    m_Threads    =  1;

    // Physics
    double m_N0         = 1.0;

    // Time
    double m_TimeStep   = 1.0;
    double m_TMin       = 0.0;
    double m_TMax       = 1.0;

};

} // End NameSpace

#endif
