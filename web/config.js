var fields = [
   ['useDHCP', 'Use DHCP', 'checkbox'],
   ['mac', 'MAC', 'text'],
   ['ip', 'IP', 'text'],
   ['subnet', 'Subnet', 'text'],
   ['gateway', 'Gateway', 'text'],
   ['dns', 'DNS', 'text'],
   ['modbus_baudrate', 'Modbus Baudrate', 'text'],
];

function addField(id, name, t)
{
    var tbl = $("#config_table");

    html = '<tr><td class="label" >' + name + ':</td>';
    html += '<td class="field" ><input name="' + id + '" id="' + id + '" type="' + t + '" style="width:135px;height:22px;"></td></tr>';
    tbl.append(html);
}

function init() {
    console.log("init called");

    for (i = 0; i < fields.length; i++) {
        f = fields[i];
        addField(f[0], f[1], f[2]);
    }

    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (this.readyState == XMLHttpRequest.DONE) {
            if (this.responseXML != null) {
                var v = this.responseXML.getElementsByTagName('config');
                v = v[0].childNodes;
                var nb = v.length;
                for (count = 0; count < nb; count++) {
                    var id = v[count].nodeName;
                    if (id == '#text') continue;
                    var f = $('#' + id);
                    if (!f) continue;
                    f = f[0];
                    if (f.type == "checkbox") {
                        f.checked = (v[count].textContent == "1");
                    }
                    else {
                        f.value = v[count].textContent;
                    }
                }
            }
        }
    }
    request.open("GET", "getconfig.xml", true);
    request.timeout = 5000;
    request.send(null);
}

function setconfig() {
    document.getElementById("form_1").submit();
}