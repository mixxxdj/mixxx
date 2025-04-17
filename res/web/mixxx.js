function readCookie(name) {
    name += '=';
    for (var ca = document.cookie.split(/;\s*/), i = ca.length - 1; i >= 0; i--)
        if (!ca[i].indexOf(name))
            return ca[i].replace(name, '');
}

function login(password){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance 
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [{
            "login":{"password" : password}
        }]
    ));
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var resp=JSON.parse(xmlhttp.responseText);
            for(var i=0; i<resp.length; ++i){
                document.cookie = "sessionid="+resp[i].sessionid +"; SameSite=Lax";
                if(resp[i].sessionid)
                    window.location.replace("/library.html");
            }
        }
    }
}

function checkauth(){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance 
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
        ]
    ));

    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var resp=JSON.parse(xmlhttp.responseText);
            for(var i=0; i<resp.length; ++i){
                if(resp[i].error=="wrong sessionid"){
                    window.location.replace("/login.html");
                }
            }
        }
    }
}

function search(searchtext){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance 
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"searchtrack": searchtext}
        ]
    ));
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var result = document.getElementById("result");
            var resjs=JSON.parse(xmlhttp.responseText);
            while (result.firstChild) {
                result.removeChild(result.firstChild);
            }
            var ttable=result.appendChild(document.createElement("table"));
            for (var i = 0; i < resjs.length; i++) {
                    var track = resjs[i].tracklist;
                    if(track){
                        var changecolor=false;
                        for(var ii = 0; ii < track.length; ii++){
                            var trow=document.createElement("tr");
                            if(changecolor)
                                trow.setAttribute("style","background:#898989;");
                            trow.insertAdjacentHTML("afterbegin","<td><input type=\"radio\" class=\"seltracks\" name=\"seltrack\" value="+track[ii].id+"></td><td>"+track[ii].artist+"</td><td>"+track[ii].title+"</td>");
                            ttable.appendChild(trow);
                            if(changecolor)
                                changecolor=false;
                            else
                                changecolor=true;
                        }
                    }
            }
        }
    }
}

function addtoautodj(position){
    var sels=document.getElementsByClassName("seltracks");
    var trackid;
    for(var i=0; i< sels.length; ++i){
        if(sels[i].checked){
            trackid=sels[i].value;
        }
    }
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"addautodj": { "trackid": trackid , "position": position}},
        ]
    ));
}

function loadautodjtracklist(){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"getautotracklist" : "true"},
        ]
    ));
    xmlhttp.onload = (event) => {
        var resjs=JSON.parse(xmlhttp.responseText);
        var autopl = document.getElementById("autoplaylist");
        var ttable=autopl.appendChild(document.createElement("table"));
        for (var i = 0; i < resjs.length; i++) {
            var track = resjs[i].tracklist;
            if(track){
                var changecolor=false;
                for(var ii = 0; ii < track.length; ii++){
                    var trow=document.createElement("tr");
                    if(changecolor)
                        trow.setAttribute("style","background:#898989;");
                    trow.insertAdjacentHTML("afterbegin","<td><input type=\"radio\" class=\"seltracks\" name=\"seltrack\" value="+track[ii].id+"></td><td>"+track[ii].artist+"</td><td>"+track[ii].title+"</td>");
                    ttable.appendChild(trow);
                    if(changecolor)
                        changecolor=false;
                    else
                        changecolor=true;
                }
            }
        }
    };
}
