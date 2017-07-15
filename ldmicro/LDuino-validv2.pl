   0:# 
   1:# ======= START RUNG 1 =======
   2:set bit '$rung_top'
   4:# start series [
   5:# start parallel [
   6:let bit '$parThis_0000' := '$rung_top'
   7:# start series [
   8:# ELEM_CONTACTS
   9:if not 'XA1' {
  10:    clear bit '$parThis_0000'
  11:}
  13:# start parallel [
  14:# ELEM_MOVE
  15:if '$parThis_0000' {
  16:    let var 'Pduty' := 50
  17:}
  19:let bit '$parThis_0001' := '$parThis_0000'
  20:# start series [
  21:# ELEM_TON
  22:if '$parThis_0001' {
  23:    if 'Tdelay' < 50 {
  24:        increment 'Tdelay'
  25:        clear bit '$parThis_0001'
  26:    }
  27:} else {
  28:    let var 'Tdelay' := 0
  29:}
  31:# ELEM_COIL
  32:let bit 'YD3' := '$parThis_0001'
  34:# ] finish series
  35:# ] finish parallel
  36:# ] finish series
  37:let bit '$parThis_0000' := '$rung_top'
  38:# start series [
  39:# ELEM_CONTACTS
  40:if 'XA1' {
  41:    clear bit '$parThis_0000'
  42:}
  44:# ELEM_MOVE
  45:if '$parThis_0000' {
  46:    let var 'Pduty' := 75
  47:}
  49:# ] finish series
  50:# ] finish parallel
  51:# ] finish series
  52:# 
  53:# ======= START RUNG 2 =======
  54:set bit '$rung_top'
  56:# start series [
  57:# ELEM_SET_PWM
  58:if '$rung_top' {
  59:    set pwm 'Pduty' 1000 Hz out P0
  60:}
  62:# ] finish series
  63:# 
  64:# ======= START RUNG 3 =======
  65:set bit '$rung_top'
  67:# start series [
  68:# ELEM_READ_ADC
  69:if '$rung_top' {
  70:    read adc 'AA0'
  71:}
  73:# ] finish series
  74:# 
  75:# ======= START RUNG 4 =======
  76:set bit '$rung_top'
  78:# start series [
  79:# ELEM_DIV
  80:if '$rung_top' {
  81:    let var '$scratch2' := 4
  82:    let var 'Padc' := 'AA0' / '$scratch2'
  83:}
  85:# ] finish series
  86:# 
  87:# ======= START RUNG 5 =======
  88:set bit '$rung_top'
  90:# start series [
  91:# ELEM_SET_PWM
  92:if '$rung_top' {
  93:    set pwm 'Padc' 1000 Hz out P1
  94:}
  96:# ] finish series
  97:# 
  98:# ======= START RUNG 6 =======
  99:set bit '$rung_top'
 101:# start series [
 102:# ELEM_CONTACTS
 103:if 'Rblink' {
 104:    clear bit '$rung_top'
 105:}
 107:# ELEM_TON
 108:if '$rung_top' {
 109:    if 'Ton' < 50 {
 110:        increment 'Ton'
 111:        clear bit '$rung_top'
 112:    }
 113:} else {
 114:    let var 'Ton' := 0
 115:}
 117:# ELEM_TOF
 118:if not '$Toff_antiglitch' {
 119:    let var 'Toff' := 15
 120:}
 121:set bit '$Toff_antiglitch'
 122:if not '$rung_top' {
 123:    if 'Toff' < 15 {
 124:        increment 'Toff'
 125:        set bit '$rung_top'
 126:    }
 127:} else {
 128:    let var 'Toff' := 0
 129:}
 131:# start parallel [
 132:let bit '$parThis_0002' := '$rung_top'
 133:# ELEM_COIL
 134:let bit 'Rblink' := '$parThis_0002'
 136:let bit '$parThis_0002' := '$rung_top'
 137:# ELEM_COIL
 138:let bit 'Mmb0' := '$parThis_0002'
 140:let bit '$parThis_0002' := '$rung_top'
 141:# ELEM_COIL
 142:let bit 'YD1' := '$parThis_0002'
 144:let bit '$parThis_0002' := '$rung_top'
 145:# start series [
 146:# ELEM_ONE_SHOT_RISING
 147:if '$parThis_0002' {
 148:    if '$oneShot_0000_ONE_SHOT_RISING_' {
 149:        clear bit '$parThis_0002'
 150:    } else {
 151:        set bit '$oneShot_0000_ONE_SHOT_RISING_'
 152:    }
 153:} else {
 154:    clear bit '$oneShot_0000_ONE_SHOT_RISING_'
 155:}
 157:# ELEM_ADD
 158:if '$parThis_0002' {
 159:    increment 'Hmb3'
 160:}
 162:# ] finish series
 163:# ] finish parallel
 164:# ] finish series
 165:# 
 166:# ======= START RUNG 7 =======
 167:set bit '$rung_top'
 169:# start series [
 170:# ELEM_CONTACTS
 171:if not 'Imb0' {
 172:    clear bit '$rung_top'
 173:}
 175:# start parallel [
 176:let bit '$parThis_0003' := '$rung_top'
 177:# ELEM_COIL
 178:let bit 'YD2' := '$parThis_0003'
 180:let bit '$parThis_0003' := '$rung_top'
 181:# ELEM_COIL
 182:let bit 'Mmb1' := '$parThis_0003'
 184:# ] finish parallel
 185:# ] finish series
 186:# 
 187:# ======= START RUNG 8 =======
 188:set bit '$rung_top'
 190:# start series [
 191:# ELEM_SET_PWM
 192:if '$rung_top' {
 193:    set pwm 'Pblink' 1000 Hz out P2
 194:}
 196:# ] finish series
 197:# 
 198:# ======= START RUNG 9 =======
 199:set bit '$rung_top'
 201:# start series [
 202:# ELEM_CONTACTS
 203:if not 'Rclock' {
 204:    clear bit '$rung_top'
 205:}
 207:# start parallel [
 208:clear bit '$parOut_0000'
 209:let bit '$parThis_0004' := '$rung_top'
 210:# ELEM_TON
 211:if '$parThis_0004' {
 212:    if 'Tnew' < 2 {
 213:        increment 'Tnew'
 214:        clear bit '$parThis_0004'
 215:    }
 216:} else {
 217:    let var 'Tnew' := 0
 218:}
 220:if '$parThis_0004' {
 221:    set bit '$parOut_0000'
 222:}
 223:let bit '$parThis_0004' := '$rung_top'
 224:# start series [
 225:# ELEM_CTC
 226:if '$parThis_0004' {
 227:    clear bit '$parThis_0004'
 228:    if not '$oneShot_0001_CTC_Ccnt' {
 229:        set bit '$oneShot_0001_CTC_Ccnt'
 230:        increment 'Ccnt'
 231:        if 'Ccnt' < 51 {
 232:        } else {
 233:            let var 'Ccnt' := 0
 234:            set bit '$parThis_0004'
 235:        }
 236:    }
 237:} else {
 238:    clear bit '$oneShot_0001_CTC_Ccnt'
 239:}
 241:# ELEM_OPEN
 242:clear bit '$parThis_0004'
 244:# ] finish series
 245:if '$parThis_0004' {
 246:    set bit '$parOut_0000'
 247:}
 248:let bit '$rung_top' := '$parOut_0000'
 249:# ] finish parallel
 250:# start parallel [
 251:let bit '$parThis_0005' := '$rung_top'
 252:# ELEM_COIL
 253:if '$parThis_0005' {
 254:    clear bit 'Rclock'
 255:} else {
 256:    set bit 'Rclock'
 257:}
 259:# ELEM_MUL
 260:if '$rung_top' {
 261:    let var '$scratch2' := 5
 262:    let var 'Pblink' := 'Ccnt' * '$scratch2'
 263:}
 265:# ] finish parallel
 266:# ] finish series
