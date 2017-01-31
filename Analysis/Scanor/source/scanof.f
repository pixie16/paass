C$PROG SCANOF    - Off-line version of SCANOR
C   
C     ******************************************************************
C     From WT Milner, JR Beene, et al at HRIBF - last modified 11/29/99
C     ******************************************************************
C
      IMPLICIT NONE
C
C     ------------------------------------------------------------------
      COMMON/LLL/ MSSG(28),NAMPROG(2),LOGUT,LOGUP,LISFLG,MSGF
      INTEGER*4   MSSG,NAMPROG,LOGUT,LOGUP
      CHARACTER*4 LISFLG,MSGF
      CHARACTER*112 CMSSG
      EQUIVALENCE (CMSSG,MSSG)
C     ------------------------------------------------------------------
      COMMON/ML01/ IWD(20),LWD(2,40),ITYP(40),NF,NTER
      INTEGER*4    IWD,    LWD,      ITYP,    NF,NTER
C     ------------------------------------------------------------------
      COMMON/ML02/ IWDRAW(20)
      INTEGER*4    IWDRAW
C     ------------------------------------------------------------------
      COMMON/SC01/ NAMCMD(20)
      INTEGER*4    NAMCMD
      CHARACTER*4  CNAMCMD(20)
      EQUIVALENCE (CNAMCMD,NAMCMD)
C     ------------------------------------------------------------------
      COMMON/SC03/ LUC(10)
      INTEGER*4    LUC
C     ------------------------------------------------------------------
      COMMON/SC04/ JCNF,IHEDN,MBFL
      INTEGER*4         IHEDN,MBFL
      CHARACTER*4  JCNF
C     ------------------------------------------------------------------
      COMMON/SC05/ NHWH,LSTL,LNBY,MAXIP,NSKIP,ISWAB,LFORM
      INTEGER*4    NHWH,LSTL,LNBY,MAXIP,NSKIP
      CHARACTER*4                             ISWAB,LFORM
C     ------------------------------------------------------------------
      COMMON/SC12/ MEM_STYLE,SHMID
      CHARACTER*80 MEM_STYLE
      INTEGER*4    SHMID
C     ------------------------------------------------------------------
      COMMON/SC13/ LCON,LCMD,LIN,LBAN,LHEP
      INTEGER*4    LCON,LCMD,LIN,LBAN,LHEP
C     ------------------------------------------------------------------
      COMMON/SC14/ NBRED,NBTOP,ICNF
      INTEGER*4    NBRED,NBTOP
      CHARACTER*4              ICNF
C     ------------------------------------------------------------------
      COMMON/SC16/ INDIR(8192),INTYP,INRECI,LUINF
      INTEGER*4    INDIR,            INRECI,LUINF
      CHARACTER*4              INTYP
C     ------------------------------------------------------------------
      COMMON/SC25/ CNAMS                   !CNAMS contains SHM filename
      CHARACTER*80 CNAMS
C     ------------------------------------------------------------------
      COMMON/ORPHAS/ STRBUFEVT,NUMBUFEVTS,BUF_NUM,LASTEVT,SUMEVTS,
     &               BEGINEVT
C
      REAL*8         STRBUFEVT,NUMBUFEVTS,BUF_NUM,LASTEVT,SUMEVTS,
     &               BEGINEVT
C     ------------------------------------------------------------------
      INTEGER*4    NAMCMDS(20)
C
      INTEGER*4    RETN,IERR,STAT,I
C   
      CHARACTER*4  IDONE,KMD,KOM
C
      EQUIVALENCE (KMD,LWD(1,1)),(KOM,IWD(1))
C
      INTEGER*4    LUT,LUH,LUD
      EQUIVALENCE (LUT,LUC(1)),
     &            (LUH,LUC(6)),
     &            (LUD,LUC(9))
C
      character*4 cnamcmds(20), ciwd(20)
      equivalence (cnamcmds, namcmds), (ciwd,iwd)
      DATA cNAMCMDS,cIWD/40*'    '/
C
C     ------------------------------------------------------------------
      SAVE
C     ------------------------------------------------------------------
C     Initialize things
C     ------------------------------------------------------------------
C
      CALL COMSET                  !Init some COMMON
C   
      CALL SCANORNIT               !Init for SCANOR
C
      GO TO 100
C   
C     ------------------------------------------------------------------
C     Test "continue flags", switch input file/dev, etc
C     ------------------------------------------------------------------
C   
C                                       !We get here via Ctrl/c
   20 ICNF='YES '                       !Set "continue flag"
      WRITE(CMSSG,25)NBRED
   25 FORMAT('NUMBER OF BUFFERS PROCESSED =',I8)
      CALL MESSLOG(LOGUT,LOGUP)
C
      IF(INTYP.EQ.'SHM ') THEN          !If SHM, report
      CALL MESSLOG(LOGUT,LOGUP)
      WRITE(CMSSG,26)
      CALL MESSLOG(LOGUT,LOGUP)
      WRITE(CMSSG,28)LASTEVT-BEGINEVT,  !#events sent
     &        SUMEVTS,                  !#events seen
     &        LASTEVT-BEGINEVT-SUMEVTS  !#events lost
      CALL MESSLOG(LOGUT,LOGUP)
      ENDIF
C
   26 FORMAT('  Total Events   Seen Events   Lost Events')
   28 FORMAT(3F14.0)
C
      CALL MESSLOG(LOGUT,LOGUP)

      GO TO 50                          !Switch to VDT input
C   
   30 LIN=LCON
      WRITE(CMSSG,35)
      CALL MESSLOG(LOGUT,LOGUP)
   35 FORMAT('Error reading cmd-file or cmd-file not assigned')
      GO TO 100
C   
   40 WRITE(CMSSG,45)
      CALL MESSLOG(LOGUT,LOGUP)
   45 FORMAT('END OF COMMAND FILE')
C   
C                                       !We get here via ERROR
   50 LIN=LCON                          !Switch to VDT input
      DO 52 I=1,20
      CNAMCMD(I)='    '
   52 CONTINUE
      CNAMCMD(1)='CON:'
      GO TO 100
C   
   55 IF(LIN.NE.LCON) GO TO 1000        !Test for VDT input
      BACKSPACE LCMD                    !Backspace cmd-file
C
   60 LIN=LCMD                          !Switch to cmd-file input
      DO 65 I=1,6                       !Restore cmd-file name
      NAMCMD(I)=NAMCMDS(I)
   65 CONTINUE
      GO TO 100
C   
   70 CALL NUINP(LCMD,IERR)             !Define new cmd-file
      DO 75 I=1,20                      !Save file-name for later
      NAMCMDS(I)=NAMCMD(I)
   75 CONTINUE
C   
  100 IF(LIN.EQ.LCON) WRITE(LOGUT,105)  !Issue prompt if in VDT mode
  105 FORMAT(' SCANOF->',$)
      MSGF='    '
C   
C     ------------------------------------------------------------------
C     Read in and process the next command from LU "LIN"
C     ------------------------------------------------------------------
C   
      READ(LIN,110,ERR=30,END=40)IWD    !Read next command from LIN
  110 FORMAT(20A4)
C
      WRITE(CMSSG,115)(IWD(I),I=1,12),(NAMCMD(I),I=1,5)
      IF(LIN.NE.LCON) CALL MESSLOG(LOGUT,LOGUP)
      IF(LIN.EQ.LCON) CALL MESSLOG(0,LOGUP)
  115 FORMAT(12A4,'     - FROM ',5A4)
C
      CALL CASEUP1(IWD)
C
      DO 120 I=1,20
      IWDRAW(I)=IWD(I)
  120 CONTINUE
C
      IF(KOM.EQ.'CMDF') GO TO 70
      IF(KOM.EQ.'CMD ') GO TO 70
      IF(KOM.EQ.'CCON') GO TO 50
      IF(KOM.EQ.'CLCM') GO TO 55
      IF(KOM.EQ.'CCMD') GO TO 60
C
      CALL CASEUP(IWD)
C
      CALL GREAD(IWD,LWD,ITYP,NF,1,80,NTER)
C
      CALL CMPSETUP(IDONE,RETN)         !Check & do setup cmds
      IF(IDONE.EQ.'YES ') THEN
      IF(RETN.EQ.50) GO TO 50
      GO TO 100
      ENDIF
C
      CALL CMPINPUT(IDONE,RETN)         !Do tape, ldf, ipc open/close
      IF(IDONE.EQ.'YES ') THEN          !record-positions, reads, etc
      IF(RETN.EQ.50) GO TO 50
      GO TO 100
      ENDIF
C
      CALL CMPREAD(IDONE,RETN)          !Check for read/display records
      IF(IDONE.EQ.'YES ') THEN
      IF(RETN.EQ.50) GO TO 50
      GO TO 100
      ENDIF
C
      IF(NTER.NE.0) GO TO 1000
C   
      IF(KMD.EQ.'ZERO') GO TO 200
      IF(KMD.EQ.'Z   ') GO TO 210
      IF(KMD.EQ.'SUM ') GO TO 220
C   
      IF(KMD.EQ.'GO  ') GO TO 250
      IF(KMD.EQ.'GOEN') GO TO 250
C   
      IF(KMD.EQ.'END ') GO TO 300
      IF(KMD.EQ.'HUP ') GO TO 320
      IF(KMD.EQ.'KILL') GO TO 340
C   
      GO TO 1000
C   
C     ------------------------------------------------------------------
C     Zero the histogram file and reset all pointers
C     ------------------------------------------------------------------
C   
  200 CALL HISNIT(LUH,'ZOT ')
      ICNF='NO  '
      JCNF='NO  '
      NBRED=0
      GO TO 100
C
C     ------------------------------------------------------------------
C     Zero individual histograms
C     ------------------------------------------------------------------
C
  210 CALL ZOTTUM(IWD)
      GO TO 100
C
C     ------------------------------------------------------------------
C     Display/log histogram sumsd
C     ------------------------------------------------------------------
C
  220 CALL HISSUM_VM
      GO TO 100
C
C     ------------------------------------------------------------------
C     Read and process records until you hit an abnormal condition
C     ------------------------------------------------------------------
C   
  250 CALL DOSCAN(RETN)
C
      IF(MSGF.NE.'    ') GO TO 20
      IF(RETN.EQ.0)      GO TO 100
      IF(RETN.EQ.20)     GO TO 20
      IF(RETN.EQ.50)     GO TO 50
      GO TO 50
C
C     ------------------------------------------------------------------
C     Process - END command
C     ------------------------------------------------------------------
C
  300 IF(LUT.LT.0) GO TO 320            !END - Tape not open
      KMD='CLOT'                        !END - Dismount no-unload
      CALL TAPOPEN(IERR)
C
C     ------------------------------------------------------------------
C     Windup - Windup - Windup - Windup - Windup - Windup - Windup
C     ------------------------------------------------------------------
C   
  320 CALL HISNIT(LUH,'HUP ')
      IF(KMD.EQ.'HUP ') GO TO 100
C
 340  continue
C
      IF((MEM_STYLE(1:5).NE.'LOCAL')) THEN  !Test for & delete 
      CALL MMAP_CLOSE(IERR)          !terminate the memory map
      ENDIF
C
CX     IF(INTYP.EQ.'SHM ') THEN              !Test  for SHM assigned
CX     CALL CLOSEIPC()                       !Detach from SHM and
CX     ENDIF
C
      STOP
C
C     ------------------------------------------------------------------
C     List/log error messages
C     ------------------------------------------------------------------
C
 1000 WRITE(CMSSG,1005)
      CALL MESSLOG(LOGUT,LOGUP)
 1005 FORMAT('Syntax error or illegal command - ignored')
      GO TO 50
C
      END
C$PROG OPENIPC   - Dummy OPENIPC for SCANOF
C
      SUBROUTINE OPENIPC(KIND,IPCNAM,LNBY,IERR)
C
      IMPLICIT NONE
C
C     ------------------------------------------------------------------
      COMMON/LLL/  MSSG(28),NAMPROG(2),LOGUT,LOGUP,LISFLG,MSGF
      INTEGER*4    MSSG,NAMPROG,LOGUT,LOGUP,LISFLG,MSGF
      CHARACTER*112 CMSSG
      EQUIVALENCE (CMSSG,MSSG)
C     ------------------------------------------------------------------
      INTEGER*4   KIND,IPCNAM,LNBY,IERR
C     ------------------------------------------------------------------
      SAVE
C     ------------------------------------------------------------------
C
      WRITE(CMSSG,10)
   10 FORMAT('----------------------------------------------')
      CALL MESSLOG(LOGUT,0)
C
      WRITE(CMSSG,20)
   20 FORMAT('This is an off-line version of SCANOR')
      CALL MESSLOG(LOGUT,0)
C
      CALL MESSLOG(LOGUT,0)
C
      WRITE(CMSSG,30)
   30 FORMAT('Connection to IPC data-stream is not supported')
      CALL MESSLOG(LOGUT,0)
C
      WRITE(CMSSG,10)
      CALL MESSLOG(LOGUT,0)
C
      IERR=1
C
      RETURN
      END
C$PROG READIPC   - Dummy READIPC for SCANOF
C
      SUBROUTINE READIPC(IBUF,NBYT,NBYRED,IERR,MSGF)
C
      IMPLICIT NONE
C
      INTEGER*4   IBUF,NBYT,NBYRED,IERR,MSGF
C
      RETURN
      END
C
C$PROG CLOSEIPC  - Dummy CLOSEIPC for SCANOF
C
      SUBROUTINE CLOSEIPC
C
      RETURN
      END
