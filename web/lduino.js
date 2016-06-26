function reboot() {
    if (!confirm("Confirm boot?")) return;
    var request = new XMLHttpRequest();
    request.open("GET", "reboot", true);
    request.timeout = 5000;
    request.send(null);
}

function popup(url) {
    newwindow = window.open(url, 'name', 'height=400,width=400,menubar=false,resizable=false,scrollbars=false, status=false, titlebar=false, toolbar=false');
    if (window.focus) { newwindow.focus() }
    return false;
}

strToggle = "";

function setLED(id, v) {
    $("#" + id + "_led").prop("checked", v ? true : false);
    $("#" + id + "_led").show();
    $("#" + id + "_led2").show();
    $("#" + id + "_lcd").hide();
    //var f = document.getElementById(id);
    //if (f) f.style.opacity = value ? 1 : 0;
    //else console.log("setLED(" + id + ") not found");
}

function setLCD(id, v) {
    $("#" + id + "_lcd").val(v);
    $("#" + id + "_lcd").show();
    $("#" + id + "_led").hide();
    $("#" + id + "_led2").hide();
}

function resetLED(id) {
    setLED(id, false);
    //var f = document.getElementById(id);
    var f = $("#" + id + "_led").get(0);
    f.onclick = function () { Toggle(this) };
}

function resetLCD(id) {
    //var f = document.getElementById(id + '_lcd');
    //f.style.visibility = "hidden";
}

function createPanelLine(id1, id2) {
    var tr = document.createElement("tr");
    tr.appendChild(createLabel(id1));
    tr.appendChild(createLCD(id1));
    tr.appendChild(createLCD(id2));
    tr.appendChild(createLabel(id2));
    return tr;
}

function IO(id)
{
    var html = "";
    html += "<td>";
    html += "<input class='LCD' id='" + id + "_lcd' maxlength='4' readonly='true' value='----' />";
    html += "<input class='LED' name='" + id + "_led' type='radio' id='" + id + "_led'/>";
    html += "<label class='LED' for='" + id + "_led' id='" + id + "_led2' radGroup1></label>";
    html += "</td>";

    return html;
}

function createSubPanel(col, prefix, nb) {
    var html = "<table>";
    for (i = 0; i < nb / 2; i++) {
        var id1 = prefix + i;
        var id2 = prefix + (i + nb / 2);
        if (id2 == "A10") id2 = "IN0";
        if (id2 == "A11") id2 = "IN1";
        html += "<tr>"
        html += "<td class='LABEL'>" + id1 + "</td>";
        html += IO(id1);
        html += IO(id2);
        html += "<td class='LABEL'>" + id2 + "</td>"
        html += "</tr>";
    }
    html += "</table>";
    cell = $("#panel tr").children("td").eq(col);
    cell.html(html);
}

function createPanel() {
    p = $("#panel tr").children("td");
    createSubPanel(0, "A", 12);
    createSubPanel(1, "D", 12);
    createSubPanel(2, "R", 10);
}

function init() {
    createPanel();

    for (i = 0; i <= 11; i++) {
        resetLED('D' + i);
        resetLCD('D' + i);
    }

    for (i = 0; i <= 9; i++) {
        resetLED('A' + i);
        resetLCD('A' + i);
    }

    for (i = 0; i <= 9; i++) {
        resetLED('R' + i);
    }

    resetLED('IN0');
    resetLED('IN1');
    resetLED('running');
    setLCD("A0", 1001);
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

function updateLCDs(xml, tag, prefix) {
    var pins = xml.getElementsByTagName(tag)[0].firstChild.nodeValue;
    var plist = pins.split(",");
    var nb = plist.length;
    for (count = 0; count < nb; count++) {
        var x = plist[count].split(":");
        setLCD(prefix + x[0], x[1]);;
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
                updateLCDs(this.responseXML, "analog", "A")
                updateLCDs(this.responseXML, "pwm", "D")

                // state
                var v = this.responseXML.getElementsByTagName('state');
                v = v[0].childNodes;
                var nb = v.length;
                for (count = 0; count < nb; count++) {
                    var n = v[count].nodeName;
                    if (n == '#text') continue;
                    if (n == strToggle) continue;
                    if (n == "running") {
                        setLED(n, (v[count].textContent == "1"));
                    }
                    else {
                        $("#" + n).val(v[count].textContent);
                    }
                }
            }
        }
    }

    var params = "";
    if (strToggle != "") {
        params += "&toggle=" + strToggle;
    }
    request.open("GET", "getstate.xml" + params, true);
    request.timeout = 5000;
    request.send(null);

    setTimeout('GetArduinoIO()', 1000);
    strToggle = "";
}

function Toggle(src) {
    strToggle =  src.id.replace("_led", "");
    console.log("Toggle " + strToggle);
}