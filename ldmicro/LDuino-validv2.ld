LDmicro0.2
MICRO=Controllino Maxi / Ext bytecode
CYCLE=10000 us at Timer1, YPlcCycleDuty:0
CRYSTAL=4000000 Hz
BAUD=2400 Hz

VAR LIST
    2 bytes Ccnt 	Now not used !!!
    2 bytes Hmb3 	Now not used !!!
    2 bytes Imb0 	Now not used !!!
    2 bytes Mmb0 	Now not used !!!
    2 bytes Mmb1 	Now not used !!!
    2 bytes Padc 	Now not used !!!
    2 bytes Pblink 	Now not used !!!
    2 bytes Pduty 	Now not used !!!
    2 bytes Rblink 	Now not used !!!
    2 bytes Rclock 	Now not used !!!
    2 bytes Tdelay 	Now not used !!!
    2 bytes Tnew 	Now not used !!!
    2 bytes Toff 	Now not used !!!
    2 bytes Ton 	Now not used !!!
END

IO LIST
    XA1 at 96 0 0
    YD1 at 7 0 0
    YD2 at 1 0 0
    YD3 at 5 0 0
    AA0 at 97 0 0
    P0 at 6 0 0
    P1 at 15 0 0
    P2 at 16 0 0
    Imb0 at 0 1 0
    Mmb0 at 0 1 0
    Mmb1 at 0 1 1
    Hmb3 at 0 1 0
END

PROGRAM
RUNG 1
  PARALLEL
    SERIES
      CONTACTS XA1 0 0
      PARALLEL
        MOVE Pduty 50
        SERIES
          TON Tdelay 500000
          COIL YD3 0 0 0 0
        END
      END
    END
    SERIES
      CONTACTS XA1 1 0
      MOVE Pduty 75
    END
  END
END
RUNG 2
  SET_PWM Pduty 1000 P0
END
RUNG 3
  READ_ADC AA0
END
RUNG 4
  DIV Padc AA0 4
END
RUNG 5
  SET_PWM Padc 1000 P1
END
RUNG 6
  CONTACTS Rblink 1 0
  TON Ton 500000
  TOF Toff 150000
  PARALLEL
    COIL Rblink 0 0 0 0
    COIL Mmb0 0 0 0 0
    COIL YD1 0 0 0 0
    SERIES
      OSR
      ADD Hmb3 Hmb3 1
    END
  END
END
RUNG 7
  CONTACTS Imb0 0 0
  PARALLEL
    COIL YD2 0 0 0 0
    COIL Mmb1 0 0 0 0
  END
END
RUNG 8
  SET_PWM Pblink 1000 P2
END
RUNG 9
  CONTACTS Rclock 0 0
  PARALLEL
    TON Tnew 20000
    SERIES
      CTC Ccnt 50 0
      OPEN
    END
  END
  PARALLEL
    COIL Rclock 1 0 0 0
    MUL Pblink Ccnt 5
  END
END
