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
            alert(xmlhttp.responseText);
        }
    }
}
