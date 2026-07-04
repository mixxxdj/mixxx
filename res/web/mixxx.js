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
                                trow.classList.add("odd");
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

function getmastergain(){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"getmastergain": "true"},
        ]
    ));
    xmlhttp.onload = (event) => {
        var resjs=JSON.parse(xmlhttp.responseText);
        for (var i = 0; i < resjs.length; i++) {
            if(resjs[i].mastergain !== undefined){
                document.getElementById("mastergainrange").value = resjs[i].mastergain;
            }
        }
    };
}

function setmastergain(value){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"setmastergain": { "gain": parseFloat(value) }},
        ]
    ));
}

function getcrossfader(){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"getcrossfader": "true"},
        ]
    ));
    xmlhttp.onload = (event) => {
        var resjs=JSON.parse(xmlhttp.responseText);
        for (var i = 0; i < resjs.length; i++) {
            if(resjs[i].crossfader !== undefined){
                document.getElementById("crossfaderrange").value = resjs[i].crossfader;
            }
        }
    };
}

function setcrossfader(value){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"setcrossfader": { "value": parseFloat(value) }},
        ]
    ));
}

function getautodjenabled(){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"getautodjenabled": "true"},
        ]
    ));
    xmlhttp.onload = (event) => {
        var resjs=JSON.parse(xmlhttp.responseText);
        for (var i = 0; i < resjs.length; i++) {
            if(resjs[i].autodjenabled !== undefined){
                document.getElementById("autodjenabled").checked = resjs[i].autodjenabled;
            }
        }
    };
}

function setautodjenabled(enabled){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"setautodjenabled": { "enabled": enabled }},
        ]
    ));
}

var deckSeeking = false;
var deckDuration = 0;

function formatTime(seconds){
    if(!isFinite(seconds) || seconds < 0){
        seconds = 0;
    }
    var mins = Math.floor(seconds / 60);
    var secs = Math.floor(seconds % 60);
    return mins + ":" + (secs < 10 ? "0" : "") + secs;
}

function getnumdecks(){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"getnumdecks": "true"},
        ]
    ));
    xmlhttp.onload = (event) => {
        var resjs=JSON.parse(xmlhttp.responseText);
        for (var i = 0; i < resjs.length; i++) {
            if(resjs[i].numdecks !== undefined){
                var select = document.getElementById("deckselect");
                select.innerHTML = "";
                for(var d=1; d<=resjs[i].numdecks; d++){
                    var opt = document.createElement("option");
                    opt.value = d;
                    opt.textContent = "Deck " + d;
                    select.appendChild(opt);
                }
                onDeckChange();
                if(!window.deckPollStarted){
                    window.deckPollStarted = true;
                    setInterval(() => {
                        if(!deckSeeking){
                            getdeckstate(document.getElementById("deckselect").value);
                        }
                    }, 1000);
                }
            }
        }
    };
}

function onDeckChange(){
    getdeckstate(document.getElementById("deckselect").value);
}

function getdeckstate(deck){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"getdeckstate": { "deck": parseInt(deck) }},
        ]
    ));
    xmlhttp.onload = (event) => {
        var resjs=JSON.parse(xmlhttp.responseText);
        for (var i = 0; i < resjs.length; i++) {
            if(resjs[i].playing !== undefined){
                updateDeckPlayButton(resjs[i].playing);
            }
            if(resjs[i].duration !== undefined){
                deckDuration = resjs[i].duration;
                document.getElementById("deckduration").textContent = formatTime(deckDuration);
            }
            if(resjs[i].position !== undefined && !deckSeeking){
                document.getElementById("deckposition").value = Math.round(resjs[i].position * 1000);
                document.getElementById("deckelapsed").textContent = formatTime(resjs[i].elapsed);
            }
        }
    };
}

function onDeckSeekInput(sliderValue){
    document.getElementById("deckelapsed").textContent =
            formatTime((sliderValue / 1000) * deckDuration);
}

function onDeckSeekCommit(sliderValue){
    var deck = document.getElementById("deckselect").value;
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"setdeckposition": { "deck": parseInt(deck), "position": sliderValue / 1000 }},
        ]
    ));
}

function updateDeckPlayButton(playing){
    var playbtn = document.getElementById("deckplaybtn");
    var pausebtn = document.getElementById("deckpausebtn");
    playbtn.classList.toggle("accent", playing);
    pausebtn.classList.toggle("accent", !playing);
}

function setDeckPlaying(playing){
    var deck = document.getElementById("deckselect").value;
    setdeckplay(deck, playing);
    updateDeckPlayButton(playing);
}

function deckstop(){
    var deck = document.getElementById("deckselect").value;
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"deckstop": { "deck": parseInt(deck) }},
        ]
    ));
    updateDeckPlayButton(false);
}

function setdeckplay(deck, playing){
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"setdeckplay": { "deck": parseInt(deck), "playing": playing }},
        ]
    ));
}

function deckcue(){
    var deck = document.getElementById("deckselect").value;
    var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance
    xmlhttp.open("POST", "/rcontrol",true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.responseType = 'text';
    xmlhttp.send(JSON.stringify(
        [
            {"sessionid": readCookie("sessionid")},
            {"deckcue": { "deck": parseInt(deck) }},
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
                        trow.classList.add("odd");
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
