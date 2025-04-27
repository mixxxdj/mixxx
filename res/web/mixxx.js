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
    xmlhttp.onload = (event) => {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var resp=JSON.parse(xmlhttp.responseText);
            for(var i=0; i<resp.length; ++i){
                if(resp[i].sessionid){
                    document.cookie = "sessionid="+resp[i].sessionid +"; SameSite=Lax";
                    window.location.replace("/index.html");
                }
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
    xmlhttp.onload = (event) => {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var resp=JSON.parse(xmlhttp.responseText);
            for(var i=0; i<resp.length; ++i){
                if(resp[i].sessionid){
                    if(resp[i].sessionid==readCookie("sessionid"))
                        return;
                }
            }
            document.cookie = "sessionid=\"\"; SameSite=Lax";
            window.location.replace("/login.html");
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
    xmlhttp.onload = (event) => {
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


function delautodj(){
    var sels=document.getElementsByClassName("seltracks");
    var position,trackid;
    for(var i=0; i< sels.length; ++i){
        if(sels[i].checked){
            position=sels[i].getAttribute('apos');
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
            {"delautodj": { "position": position, "trackid": trackid }},
        ]
    ));
     xmlhttp.onload = (event) => {
        window.location.replace("/index.html");
     }
}

function moveautotracklist(direction){
    let sels=document.getElementsByClassName("seltracks");
    let position=0,newpos=0;
    for(var i=0; i< sels.length; ++i){
        if(sels[i].checked){
            position=sels[i].getAttribute('apos');
            break;
        }
    }

    newpos=position;

    let xmlhttp = new XMLHttpRequest();
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';

    if(arguments[0]=='up'){
        --newpos;
    }else if(arguments[0]=='down'){
        ++newpos;
    }
    xmlhttp.send(JSON.stringify(
            [
                {"sessionid": readCookie("sessionid")},
                {"moveautotracklist": { "position": position, "newposition" : newpos}},
            ]
    ));
    xmlhttp.onload = (event) => {
        window.location.reload();
    }
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
                    trow.insertAdjacentHTML("afterbegin","<td><input type=\"radio\" class=\"seltracks\" name=\"seltrack\" apos="+
                                                            track[ii].position+" value="+track[ii].id+"></td><td>"+track[ii].artist+"</td><td>"+track[ii].title+"</td>");
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
