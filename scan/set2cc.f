C     ******************************************************************
C
C     ******************************************************************
C     BY J.R. BEENE AT HRIBF - LAST MODIFIED by WT MILNER 02/17/99
C     ******************************************************************
C  
      SUBROUTINE SET2CC(ID,IX,IY,IZ)
C
C     ------------------------------------------------------------------
C     COUNT 1 CHECK and COMPRESS (1D and 2D)
C     ROUTINE TO ADD ONE COUNT PER CALL TO MEMORY HISTOGRAMS
C     DIFFERS FROM COUNT1 IN THAT COMPRESSION AND RANGE CHECKING
C     ARE DONE. IX,IY ARE RAW PARAMETER VALUES.
C     !!!NOTE!!! requests to increment nonexistant histograms are
C     igored without comment.
C     ------------------------------------------------------------------
C
      IMPLICIT NONE
C
C     ------------------------------------------------------------------
      COMMON/SC17/ IOFF(8000),IOFH(8000),NDIM(8000),NHPC(8000),
     &             LENX(8000),LENH(8000)
C
      INTEGER*2    LENX,                 NDIM,      NHPC
      INTEGER*4    IOFF,      IOFH
      INTEGER*4               LENH
C     ------------------------------------------------------------------
      COMMON/SC18/ ICMP(4,8000),IMIN(4,8000),IMAX(4,8000),MAXOFF
C
      INTEGER*2    ICMP,        IMIN,        IMAX
      INTEGER*4                                           MAXOFF
C     ------------------------------------------------------------------
      INTEGER*4    ID,IX,IY,ICX,ICY,IC,NDX,IZ 
C     ------------------------------------------------------------------
C
      IF(NDIM(ID).LE.0)RETURN                !Check existance
C
C
C     ICX=RSHIFT(IX,ICMP(1,ID))              !COMPRESS DEC fortran

      ICX=ISHFT(IX,-ICMP(1,ID))              !COMPRESS ansi fortran
C
C                                            ! CHECK X RANGE
      IF(ICX.LT.IMIN(1,ID).OR.ICX.GT.IMAX(1,ID))RETURN
      ICX=ICX-IMIN(1,ID)
      IC=ICX
      IF(NDIM(ID).EQ.2)THEN
C     ICY=RSHIFT(IY,ICMP(2,ID))              !COMPRESS DEC fortran
      ICY=ISHFT(IY,-ICMP(2,ID))              !COMPRESS ansi fortran
C
C                                            ! CHECK Y RANGE
      IF(ICY.LT.IMIN(2,ID).OR.ICY.GT.IMAX(2,ID))RETURN
      ICY=ICY-IMIN(2,ID)
      IC=ICY*LENX(ID)+ICX                    !CHAN-OFF FOR 2-D
      ENDIF
C
      IF(NHPC(ID).EQ.2) THEN                 !TST FOR FULL-WD CHAN
      NDX=IOFF(ID)+IC                        !FULL-WD INDEX
c      CALL MEM_ADD1_FW(NDX)                  !FULL-WD ADD-ONE
      CALL MEM_SET_VALUE_FW(NDX,IZ)               !HALF-WD ADD-ONE
      RETURN
      ELSE
C
      NDX=IOFH(ID)+IC                        !HALF-WD INDEX
        CALL MEM_SET_VALUE_HW(NDX,IZ)               !HALF-WD ADD-ONE
c      CALL MEM_ADD1_HW(NDX)                  !HALF-WD ADD-ONE
      ENDIF
C
      RETURN
      END

C     ******************************************************************
C
C     ******************************************************************
C     BY J.R. BEENE AT HRIBF - LAST MODIFIED by WT MILNER 02/17/99
C     ******************************************************************
C  
      SUBROUTINE GET2CC(ID,IX,IY,IZ)
c     
C
C     ------------------------------------------------------------------
C     COUNT 1 CHECK and COMPRESS (1D and 2D)
C     ROUTINE TO ADD ONE COUNT PER CALL TO MEMORY HISTOGRAMS
C     DIFFERS FROM COUNT1 IN THAT COMPRESSION AND RANGE CHECKING
C     ARE DONE. IX,IY ARE RAW PARAMETER VALUES.
C     !!!NOTE!!! requests to increment nonexistant histograms are
C     igored without comment.
C     ------------------------------------------------------------------
C
      IMPLICIT NONE
C
C     ------------------------------------------------------------------
      COMMON/SC17/ IOFF(8000),IOFH(8000),NDIM(8000),NHPC(8000),
     &             LENX(8000),LENH(8000)
C
      INTEGER*2    LENX,                 NDIM,      NHPC
      INTEGER*4    IOFF,      IOFH
      INTEGER*4               LENH
C     ------------------------------------------------------------------
      COMMON/SC18/ ICMP(4,8000),IMIN(4,8000),IMAX(4,8000),MAXOFF
C
      INTEGER*2    ICMP,        IMIN,        IMAX
      INTEGER*4                                           MAXOFF
C     ------------------------------------------------------------------
      INTEGER*4    ID,IX,IY,ICX,ICY,IC,NDX,IZ
C     ------------------------------------------------------------------

      INTEGER*2    MEM_GET_VALUE_HW

      INTEGER*4    MEM_GET_VALUE_FW
C
      IF(NDIM(ID).LE.0)RETURN                !Check existance
C
C
C     ICX=RSHIFT(IX,ICMP(1,ID))              !COMPRESS DEC fortran

      ICX=ISHFT(IX,-ICMP(1,ID))              !COMPRESS ansi fortran
C
C                                            ! CHECK X RANGE
      IF(ICX.LT.IMIN(1,ID).OR.ICX.GT.IMAX(1,ID))RETURN
      ICX=ICX-IMIN(1,ID)
      IC=ICX
      IF(NDIM(ID).EQ.2)THEN
C     ICY=RSHIFT(IY,ICMP(2,ID))              !COMPRESS DEC fortran
      ICY=ISHFT(IY,-ICMP(2,ID))              !COMPRESS ansi fortran
C
C                                            ! CHECK Y RANGE
      IF(ICY.LT.IMIN(2,ID).OR.ICY.GT.IMAX(2,ID))RETURN
      ICY=ICY-IMIN(2,ID)
      IC=ICY*LENX(ID)+ICX                    !CHAN-OFF FOR 2-D
      ENDIF
C
      IF(NHPC(ID).EQ.2) THEN                 !TST FOR FULL-WD CHAN
      NDX=IOFF(ID)+IC                        !FULL-WD INDEX
c      CALL MEM_ADD1_FW(NDX)                  !FULL-WD ADD-ONE
      IZ=MEM_GET_VALUE_FW(NDX)               !HALF-WD ADD-ONE
c      print *,iz,'fw'
      RETURN
      ELSE
C
      NDX=IOFH(ID)+IC                        !HALF-WD INDEX
      IZ=MEM_GET_VALUE_HW(NDX)               !HALF-WD ADD-ONE
      print *,iz,'hw'
c      CALL MEM_ADD1_HW(NDX)                  !HALF-WD ADD-ONE
      ENDIF
C
      RETURN
      END

c===============================================================================
