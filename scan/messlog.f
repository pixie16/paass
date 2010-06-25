C$PROG MESSLOG   - General message logger with 4-digit yr 
C
C     ******************************************************************
C     BY W.T. MILNER AT HRIBF - LAST MODIFIED 06/19/2002 - for gnu
C     ******************************************************************
C
      SUBROUTINE MESSLOG(LUA,LUB)
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
      INTEGER*4 LUA,LUB,N,I,NTRY

C     UNUSED VARIABLES
C     INTEGER*4 YR2,YR4
C
      INTEGER*4    DATIM(5),MSS20(20)
C
      EQUIVALENCE  (MSS20,MSSG)
C     ------------------------------------------------------------------
      INTEGER*4    BLANK
      CHARACTER*4  CBLANK
C
      EQUIVALENCE  (BLANK,CBLANK)
      DATA         CBLANK/'    '/
C
      SAVE
C
C     ------------------------------------------------------------------
C
      IF(LUA.LE.0) GO TO 20
C
      WRITE(LUA,10)MSS20
   10 FORMAT(1H ,19A4,A3)
C
   20 IF(LUB.LE.0.OR.LISFLG.NE.'LON ') GO TO 100
C
      CALL MILDATIM(DATIM)
C
      NTRY=0
      N=1
      DO 40 I=1,26
      IF(MSSG(I).NE.BLANK) N=I
   40 CONTINUE
C

      CALL FSEEK(LUB,0,2)
C
   50 WRITE(LUB,55,ERR=60)NAMPROG,DATIM,(MSSG(I),I=1,N)
   55 FORMAT(A4,A3,5A4,1X,26A4)
      GO TO 100
C
   60 NTRY=NTRY+1
      IF(NTRY.LT.10) GO TO 50
      WRITE(LOGUT,65)
   65 FORMAT(1H ,'10 FAILURES WRITING FOLLOWING TO LOG-FILE')
      WRITE(LOGUT,70)NAMPROG,DATIM
   70 FORMAT(1H ,A4,A3,5A4)
      WRITE(LOGUT,10)MSS20
C
  100 CMSSG=' '
C
      RETURN
      END
