strToggle = "";

function setLED(id, value) {
    var f = document.getElementById(id);
    if (f) f.style.opacity = value ? 1 : 0;
    else console.log("setLED(" + id + ") not found");
}

function initLED(id) {
    setLED(id, false);
    var f = document.getElementById(id);
    f.onclick = function () { Toggle(this) };
}

function init() {
    for (i = 0; i <= 11; i++) {
        initLED('D' + i);
    }

    for (i = 0; i <= 9; i++) {
        initLED('A' + i);
    }

    for (i = 0; i <= 9; i++) {
        initLED('R' + i);
    }

    initLED('IN0');
    initLED('IN1');
    initLED('running');

    GetArduinoIO();
}

function updateLEDs(xml, tag, prefix) {
    var pins = xml.getElementsByTagName(tag)[0].firstChild.nodeValue;
    var plist = pins.split(",");
    var nb = plist.length;
    for (count = 0; count < nb; count++) {
        var x = plist[count].split(":");
        setLED(prefix + x[0], x[1] > 0);
    }
}

function GetArduinoIO() {
    nocache = "&nocache=" + Math.random() * 1000000;
    var request = new XMLHttpRequest();

    request.onreadystatechange = function () {
        if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
            if (this.responseXML != null) {
                // XML file received - contains analog values, switch values and LED states
                var count;

                // IO pins state
                updateLEDs(this.responseXML, "outputs", "D");
                updateLEDs(this.responseXML, "relays", "R");
                updateLEDs(this.responseXML, "inputs", "A");

                // state
                var v = this.responseXML.getElementsByTagName('state');
                v = v[0].childNodes;
                var nb = v.length;
                for (count = 0; count < nb; count++) {
                    var n = v[count].nodeName;
                    if (n == '#text') continue;
                    var f = document.getElementById(n);
                    if (!f) continue;
                    if (n == "running") {
                        f.style.opacity = (v[count].textContent == "1") ? 1 : 0;
                    }
                    else {
                        f.lastElementChild.childNodes[0].data = v[count].textContent;
                    }
                }
            }
        }
    }
    
    request.open("GET", "getstate.xml" + strToggle, true);
    request.timeout = 5000;
    request.send(null);

    setTimeout('GetArduinoIO()', 1000);
    strToggle = "";
}

function Toggle(src) {
    console.log("Toggle " + src.id);
    strToggle = "&toggle=" + src.id;
}