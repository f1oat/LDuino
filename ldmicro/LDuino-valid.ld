LDmicro0.1
MICRO=Controllino Maxi / Ext bytecode
CYCLE=10000 us at Timer1, YPlcCycleDuty:0
CRYSTAL=4000000 Hz
BAUD=2400 Hz

IO LIST
    XA1 at 96 0 0
    YD1 at 7 0 0
    YD2 at 1 0 0
    YD3 at 5 0 0
    AA0 at 97 0 0
END

PROGRAM
RUNG
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
RUNG
  SET_PWM Pduty 1000 P0
END
RUNG
  READ_ADC AA0
END
RUNG
  DIV Padc AA0 4
END
RUNG
  SET_PWM Padc 1000 P1
END
RUNG
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
RUNG
  CONTACTS Imb0 0 0
  PARALLEL
    COIL YD2 0 0 0 0
    COIL Mmb1 0 0 0 0
  END
END
RUNG
  SET_PWM Pblink 1000 P2
END
RUNG
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
