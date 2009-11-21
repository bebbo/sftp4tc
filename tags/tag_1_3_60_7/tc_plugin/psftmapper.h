#ifndef __PSFTMAPPER_H__
#define __PSFTMAPPER_H__

struct PsftpMapper {
    HMODULE hDll;
    PsftpConnectProcType connect;
    PsftpDoSftpProcType doSftp;
    PsftpGetCurrentDirStructProcType getCurrentDirStruct;
    FARPROC disconnect;
    //	FARPROC SFTP_DLL_FNCT_psftp_memory_hole__stopfen;
    PsftpGetLastErrorMessageProcType getLastErrorMessage;
    PsftpInitProcsProcType initProgressProc;
    PsftpSetTransferModeProcType setTransferMode;
    PsftpDisconnectedProcType disconnected;
};

#endif // __PSFTMAPPER_H__
