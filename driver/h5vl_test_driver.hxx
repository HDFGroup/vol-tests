#ifndef H5VL_TEST_DRIVER_H
#define H5VL_TEST_DRIVER_H

#include <string>
#include <vector>

#include <h5vl_test_sys/Process.h>

class H5VLTestDriver {
public:
    int Main(int argc, char *argv[]);
    H5VLTestDriver();
    ~H5VLTestDriver();

protected:
    void SeparateArguments(const char* str, std::vector<std::string> &flags);

    void ReportCommand(const char * const *command, const char *name);
    int  ReportStatus(h5vl_test_sysProcess *process, const char *name);
    int  ProcessCommandLine(int argc, char *argv[]);
    void CollectConfiguredOptions();
    void CreateCommandLine(std::vector<const char *> &commandLine,
        const char *cmd, int isServer, int isHelper, const char *numProc,
        int argStart = 0, int argCount = 0, char *argv[] = 0);

    int StartServer(h5vl_test_sysProcess *server, const char *name,
        std::vector<char> &out, std::vector<char> &err);
    int StartClient(h5vl_test_sysProcess *client, const char *name);
    void Stop(h5vl_test_sysProcess *p, const char *name);
    int OutputStringHasError(const char *pname, std::string &output);

    int WaitForLine(h5vl_test_sysProcess *process, std::string &line,
        double timeout, std::vector<char> &out, std::vector<char> &err);
    void PrintLine(const char *pname, const char *line);
    int WaitForAndPrintLine(const char *pname, h5vl_test_sysProcess *process,
        std::string &line, double timeout, std::vector<char> &out,
        std::vector<char> &err, int *foundWaiting);

    std::string GetDirectory(std::string location);

private:
    std::string ClientExecutable;       // fullpath to client executable
    std::string ClientHelperExecutable; // fullpath to client helper executable
    std::string ServerExecutable;       // fullpath to server executable
    std::string MPIRun;                 // fullpath to mpirun executable

    // This specify the preflags and post flags that can be set using:
    // VTK_MPI_PRENUMPROC_FLAGS VTK_MPI_PREFLAGS / VTK_MPI_POSTFLAGS at config time
    // std::vector<std::string> MPIPreNumProcFlags;
    std::vector<std::string> ClientEnvVars;
    std::vector<std::string> MPIClientPreFlags;
    std::vector<std::string> MPIClientPostFlags;
    std::vector<std::string> MPIServerPreFlags;
    std::vector<std::string> MPIServerPostFlags;

    // Specify the number of process flag, this can be set using: VTK_MPI_NUMPROC_FLAG.
    // This is then split into :
    // MPIServerNumProcessFlag & MPIRenderServerNumProcessFlag
    std::string MPINumProcessFlag;
    std::string MPIServerNumProcessFlag;
    std::string MPIClientNumProcessFlag;

    std::string CurrentPrintLineName;

    double TimeOut;
    double ServerExitTimeOut;   // time to wait for servers to finish.
    bool ClientHelper;
    bool TestServer;

    int ClientArgStart;
    int ClientArgCount;
    int ClientHelperArgStart;
    int ClientHelperArgCount;
    int ServerArgStart;
    int ServerArgCount;
    bool AllowErrorInOutput;
    bool TestSerial;
};

#endif //H5VL_TEST_DRIVER_H
