#ifndef __OTA_SERVER_H__
#define __OTA_SERVER_H__

#include<Arduino.h>
#include<stdint.h>

class SBSession{
  // 10 random session msg digest.
  String SessionKeys[10] = {"eTL6mxjVE5uj35JtIvjwqMwxkf", "bzOV8kY4L8zAvhIKGOEp99MMxY", "SBmnE2C1tNwrLv4qX7QD3Sz2Rv", "HZTI7WAlGblxE2zDwo7UxFAtP1", "69ZHCSxAI0duiP1skerlklNVuL", 
                            "1f2DWFGp5WFh7f41umrJJPXRZu", "aQnOJ9z1QSV6bHloSDH1TMO84b", "WLfm7BqahusbpK90kBraC6n2U6", "LNiz7znfFyFtgxYY9StExUrKdU", "bD2HmNNlxbn07M8TJxm7fpiWaH"};
  int randSessionIndex;

  public:
    void genKey();
    String getKey(String);
};

// Function to select a random number from 0 to 9 (both inclusive)
void SBSession::genKey(){
   randSessionIndex = (int)random(0,10);
}

// Function to generate the session key and return the hash
String SBSession::getKey(String payload){
  String hashStr = SessionKeys[randSessionIndex];
  int append = strlen(payload.c_str()) % 256;
  char hex[3];
  sprintf(hex,"%02X",append);
  hashStr = "$2a$10$"+hashStr+"/"+hex;
  return hashStr;
}

// CSS style for all routes
String style = "<style>\n\
                  * {\n\
                      box-sizing: border-box;position: relative;\n\
                  }\n\
                  html,body{\n\
                      width:100%;position: relative;margin: 0;padding: 0;\n\
                      text-align:center;font-family: 'Poppins';overflow: auto;\n\
                      background: #333;min-width: 380px !important;\n\
                  }\n\
                  .container{\n\
                      font-family: 'Poppins';position: relative;\n\
                      width:100%;background: #333;text-align: center;\n\
                      z-index: 2;display: flex;padding:15px;justify-content: center;\n\
                  }\n\
                  .loginWrapper{\n\
                      min-width:350px;padding:20px 50px;background: #fff;\n\
                      border-radius: 10px;margin:50px 15px;margin-top: 0;\n\
                      max-width:500px;flex: 50%;\n\
                  }\n\
                  .loginWrapper h2{\n\
                      margin-bottom: 10px;user-select: none;color:#777;\n\
                  }\n\
                  .pinContainer{\n\
                      display: block;width:100%;margin-top:20px;\n\
                      margin-left: auto;margin-right: auto;text-align: left;\n\
                  }\n\
                  .pinContainer div h4{\n\
                      margin:0 !important;\n\
                  }\n\
                  #file-input {\n\
                      display: block;border: 1px solid #bbb;\n\
                      padding: 10px;border-radius: 5px;\n\
                      text-align: left;color: #aaa;width: 100%;\n\
                      cursor: pointer;transition: all 0.5s;\n\
                  }\n\
                  #file-input:hover {\n\
                      border-color: #777;color: #777;\n\
                  }\n\
                  input[type=\"text\"],input[type=\"password\"] {\n\
                      display: block;border: 1px solid #bbb;padding: 10px;\n\
                      border-radius: 5px;text-align: left;color: #aaa;\n\
                      width: 100%;transition: all 0.5s;font-size: 18px;\n\
                      outline: none;margin-bottom: 10px;\n\
                  }\n\
                  input[type=\"text\"]:hover,input[type=\"password\"]:hover {\n\
                      border-color: #777;color: #777;\n\
                  }\n\
                  input[type=\"text\"]:focus,input[type=\"password\"]:focus {\n\
                      border-color: #3498db;border-width: 2px;\n\
                  }\n\
                  .btn{\n\
                      background:#3498db;color:#fff;padding:15px 10px;\n\
                      width:100%;border-radius: 5px;margin-top:15px;font-size:18px;\n\
                      font-weight:bold;transition:all 0.5s;border:none;\n\
                      outline: none;cursor: pointer;\n\
                  }\n\
                  .btn:hover{\n\
                      background:rgba(52, 152, 219, 0.8);\n\
                  }\n\
                  .btn:disabled{\n\
                      cursor: not-allowed;\n\
                  }\n\
                  #prg{\n\
                      font-size:15px;color:#777;user-select: none;margin-bottom:7px;text-align:center\n\
                  }\n\
                  #bar,#prgbar{\n\
                      background-color:#f1f1f1;\n\
                      border-radius:10px\n\
                  }\n\
                  #bar{\n\
                      background-color:#3498db;\n\
                      width:0%;height:10px\n\
                  }\n\
                  .svg_container svg{\n\
                      width:100px;\n\
                      height:90px;\n\
                  }\n\
                  .window {\n\
                      display: none;width:80%;min-width:380px;\n\
                      padding:0px 30px;background: #fff;\n\
                      border-radius: 10px;position: absolute;\n\
                      top:50%;left:50%;transform:translate(-50%,-50%);\n\
                      z-index: 5;box-shadow: 0px 0px 20px 0px rgba(0,0,0,0.5);\n\
                  }\n\
                  .consoleWindow{\n\
                      background: #000;width:100%;\n\
                      color: #fff;display: block;\n\
                      text-align: left;padding: 10px;\n\
                      margin-bottom: 1.5em;font-size: 15px;\n\
                      font-family: consolas;overflow-x: auto;\n\
                  }\n\
                  .con_info{\n\
                      margin: 10px;\n\
                  }\n\
                  .ckere {\n\
                      cursor: pointer;font-weight: bold;transition: all 0.5s;\n\
                  }\n\
                  .ckere:hover {\n\
                      text-decoration: underline;\n\
                  }\n\
                  @media(max-width:765px){\n\
                      .container{flex-direction:column;}\n\
                      .loginWrapper{margin-left:auto !important;margin-right:auto !important;}\n\
                  }\n\
                  @media(max-width:650px){\n\
                      .loginWrapper{padding:20px;}\n\
                      .window{ padding:0px 18px;}\n\
                  }\n\
              </style>\n\
            </head>";

// <Body> for root route
String body = " <body> \n\
  <div style=\"padding:10px 50px\">\n\
    <a href=\"/login?LOGOUT=YES\">\n\
        <input type=\"submit\" class=\"btn\" value=\"Log Out\" style=\"width:150px\">\n\
    </a>\n\
  </div>\n\
  <hr><br>\n\
  <div class=\"container\"> <div class=\"loginWrapper\"> <div style=\"position: relative;margin: auto;width: 100%;\"> <div class=\"svg_container\">\
  <svg version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 512.000000 512.000000\" preserveAspectRatio=\"xMidYMid meet\">\
    <g transform=\"translate(0.000000,512.000000) scale(0.100000,-0.100000)\" fill=\"#777\" stroke=\"none\">\
      <path d=\"M1504 4950 c-12 -4 -31 -21 -43 -36 -20 -26 -21 -35 -21 -351 l0\
      -325 29 -29 c58 -58 158 -31 181 49 8 24 10 138 8 332 -3 323 -4 326 -64 355\
      -35 17 -59 18 -90 5z\"/>\
      <path d=\"M2093 4940 c-17 -10 -37 -28 -42 -39 -7 -13 -11 -130 -11 -331 0\
      -330 3 -355 49 -380 39 -20 98 -12 133 19 l33 29 3 311 c3 351 0 370 -62 396\
      -47 20 -65 19 -103 -5z\"/>\
      <path d=\"M2692 4940 c-18 -11 -37 -32 -42 -46 -14 -37 -13 -629 1 -657 20 -39\
      51 -57 99 -56 48 0 66 9 93 49 15 22 17 59 17 333 0 338 -2 352 -60 382 -39\
      20 -69 19 -108 -5z\"/>\
      <path d=\"M1320 3973 c-28 -10 -66 -37 -101 -72 -94 -95 -89 -38 -89 -936 l0\
      -782 30 -61 c35 -68 82 -115 149 -146 44 -20 62 -21 515 -24 410 -2 468 -1\
      463 12 -27 70 -28 106 -2 169 7 16 -18 17 -445 17 l-452 0 -29 29 -29 29 0\
      763 0 763 27 28 27 28 762 0 763 0 28 -24 28 -24 3 -457 c2 -428 4 -456 20\
      -449 36 16 119 16 151 -1 l32 -16 -3 473 c-3 457 -4 475 -24 519 -31 67 -78\
      114 -146 149 l-61 30 -786 -1 c-691 0 -791 -3 -831 -16z\"/>\
      <path d=\"M3480 3680 c-71 -5 -88 -13 -113 -50 -31 -46 -16 -114 32 -147 23\
      -16 59 -18 321 -21 194 -2 308 0 332 8 115 34 100 208 -19 211 -171 4 -500 3\
      -553 -1z\"/>\
      <path d=\"M201 3654 c-38 -32 -49 -80 -27 -126 30 -62 31 -63 356 -66 194 -2\
      308 0 332 8 80 23 107 123 49 181 l-29 29 -325 0 -325 0 -31 -26z\"/>\
      <path d=\"M1654 3543 c-12 -2 -34 -18 -50 -34 l-29 -30 -3 -482 c-2 -333 0\
      -494 8 -520 7 -25 22 -45 43 -57 31 -19 54 -20 530 -20 l499 0 34 34 34 34 0\
      499 c0 378 -3 504 -13 523 -7 14 -28 33 -47 42 -32 16 -76 18 -510 17 -261 -1\
      -485 -4 -496 -6z\"/>\
      <path d=\"M200 3051 c-37 -37 -46 -73 -28 -119 26 -70 40 -73 396 -70 l314 3\
      29 33 c41 46 41 111 0 153 l-29 29 -327 0 -327 0 -28 -29z\"/>\
      <path d=\"M3482 2827 c-6 -7 -20 -69 -32 -137 -12 -69 -22 -126 -24 -127 -1 -1\
      -39 -13 -85 -26 l-82 -24 -86 104 c-54 64 -93 103 -102 101 -26 -5 -230 -122\
      -235 -135 -3 -7 16 -68 41 -138 l46 -125 -61 -62 -62 -61 -125 46 c-70 25\
      -131 44 -138 41 -15 -6 -137 -218 -137 -237 0 -8 47 -53 104 -100 l103 -86\
      -24 -82 c-13 -46 -24 -84 -26 -85 -1 -1 -58 -12 -127 -24 -69 -12 -131 -27\
      -137 -32 -19 -15 -18 -261 0 -276 7 -6 69 -21 138 -33 68 -12 125 -23 127 -24\
      5 -4 42 -144 42 -159 0 -8 -45 -52 -100 -97 -55 -46 -100 -89 -100 -96 0 -19\
      130 -243 140 -243 5 0 65 21 133 46 l125 46 62 -62 62 -62 -45 -126 c-25 -69\
      -43 -130 -41 -136 6 -15 217 -136 237 -136 9 0 53 45 98 100 46 55 86 100 90\
      100 8 0 162 -40 165 -43 1 -1 12 -58 24 -127 12 -69 26 -131 32 -137 15 -19\
      261 -18 276 0 6 7 21 69 33 138 12 68 23 125 24 126 2 4 156 43 165 43 4 0 45\
      -45 90 -100 46 -55 90 -100 98 -100 13 0 188 95 227 124 18 13 17 18 -29 143\
      l-47 130 62 63 62 62 130 -48 c125 -46 130 -47 143 -29 30 41 124 214 124 228\
      0 8 -45 52 -100 97 -55 45 -100 86 -100 90 0 9 39 163 43 165 1 2 47 10 102\
      19 55 9 117 21 138 27 l37 10 0 133 c0 95 -4 137 -12 144 -7 6 -69 20 -138 32\
      -69 12 -126 23 -127 24 -3 3 -43 157 -43 165 0 4 45 44 100 90 55 45 100 89\
      100 98 0 18 -117 224 -133 235 -6 3 -68 -14 -138 -39 l-127 -45 -62 62 -62 62\
      46 125 c25 68 46 128 46 133 0 10 -224 140 -243 140 -7 0 -50 -45 -96 -100\
      -45 -55 -88 -100 -94 -100 -15 0 -158 37 -162 42 -1 2 -12 59 -24 127 -12 69\
      -27 131 -33 138 -15 18 -261 19 -276 0z m266 -711 c3 -13 -21 -44 -74 -97\
      l-79 -79 263 -263 c166 -166 262 -269 262 -282 0 -27 -368 -395 -395 -395 -13\
      0 -114 94 -282 262 l-263 263 -79 -79 c-123 -122 -116 -140 -119 298 -2 266 1\
      373 9 382 10 12 77 14 383 12 354 -3 371 -4 374 -22z\"/>\
      <path d=\"M220 2467 c-14 -7 -33 -29 -44 -51 -25 -51 -10 -108 37 -136 30 -19\
      52 -20 342 -20 276 0 313 2 335 17 40 27 49 45 49 93 1 48 -17 79 -56 99 -32\
      16 -629 15 -663 -2z\"/>\
      <path d=\"M1488 1753 c-14 -9 -31 -27 -37 -40 -7 -16 -11 -137 -11 -344 l0\
      -321 29 -29 c24 -23 38 -29 73 -29 52 0 85 21 104 66 17 42 20 576 3 637 -18\
      67 -101 97 -161 60z\"/>\
    </g>\
  </svg>\
  </div> <div style=\"display: block;width:100%\"><h2>SmartBin</h2></div> <div class=\"pinContainer\"> <form method=\"POST\" action=\"\" enctype=\"multipart/form-data\" id=\"upload_form\"> <input type=\"file\" name=\"update\" id=\"file\" onchange=\"sub(this)\" style=\"display:none\"> <label id=\"file-input\" for=\"file\"> Choose file...</label> <input type=\"submit\" class=\"btn\" id=\"btn\" value=\"Upload\" disabled> <br><br> <div id=\"prg\"></div> <div id=\"prgbar\"> <div id=\"bar\"></div> </div> </form> </div> <div class=\"helpContainer\"></div> </div> </div></div> <div class=\"window\" id=\"cmdContainer\"> <div style=\"display: block;\"><h3>Console Log</h3></div> <div class=\"consoleWindow\" id=\"consoleWindowCont\"></div> <div class=\"con_info\">Click <label class=\"ckere\" onclick=\"closeConsole();\">here</label> to close this window. After you close, the page will automatically reload in 2 seconds</div> </div>";

// <script> for rooot route
String script = "<script> function closeConsole(){ document.getElementById(\"cmdContainer\").style.display=\"none\"; setTimeout(function(){ location.reload(); },3000); } function processResponse(res){ let ans=\"\"; for(i=0;i<res.length;i++){ if(res[i]==\" \"){ ans+=\"&nbsp;\"; } else if(res[i]==\"\\n\"){ ans+=\"<br>\"; } else{ ans+=res[i]; } } return ans; } function sub(obj){ var file = obj.files; if(file.length == 0){ alert(\"No file selected\"); return; } else { var name = file[0].name; var extensionList = name.split('.'); var extension = extensionList[extensionList.length-1].toLowerCase(); if(extension != \"hex\"){ alert(\"Only HEX files are accepted. Please choose a valid HEX file.\"); return; } } document.getElementById(\"file-input\").innerHTML = \" \"+ file[0].name; document.getElementById(\"btn\").disabled = false; }; document.getElementById(\"upload_form\").onsubmit = function(event) { event.preventDefault(); let files = document.getElementById(\"file\").files; let file = files[0]; let formData = new FormData(); formData.append('file', file); console.log(formData); let xmlhttp=\"\"; if(window.XMLHttpRequest) { xmlhttp=new XMLHttpRequest(); } else { xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\"); } xmlhttp.onreadystatechange = function() { if (xmlhttp.readyState == 4) { if (xmlhttp.status == 200) { console.log(xmlhttp.responseText); document.getElementById(\"prg\").innerHTML = \"Uploaded Succesfully\"; document.getElementById(\"cmdContainer\").style.display=\"block\"; document.getElementById(\"consoleWindowCont\").innerHTML=processResponse(xmlhttp.responseText); } else if (xmlhttp.status == 0) { alert(\"Server closed the connection abruptly!\"); setTimeout(function(){ alert(\"Reloading Now...\"); location.reload(); },1500); } else { console.log(xmlhttp.status + \" Error!\\n\" + xmlhttp.responseText); document.getElementById(\"prg\").innerHTML = \"Error in Uploading\"; setTimeout(function(){ alert(\"Reloading Now...\"); location.reload(); },1500); } } }; xmlhttp.upload.addEventListener('progress', function(evt) { if (evt.lengthComputable) { let per = evt.loaded / evt.total; document.getElementById(\"prg\").innerHTML = \"Progress: \" + Math.round(per*100) + \"%\"; document.getElementById(\"bar\").style.width = Math.round(per*100) + \"%\"; } else{ console.log(\"Unable to compute progress information since the total size is unknown\"); } }); xmlhttp.open(\"POST\", \"upload\", true); xmlhttp.send(formData); } </script>";

String end = " </body> </html>";

// Function to get the the header of <html> based on route
String getHeadTitle(String title){
  return "<!doctype html>\n\
            <html>\n\
            <head>\n\
                <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n\
                <meta charset=\"UTF-8\">\n\
                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
                <title>"+title+"</title>";
}

// Full root route page
String root(){ return getHeadTitle("FYP: SmartBin")+style+body+script+end;}

// <body> for login route
String getLoginBody(String msg){
  String err = "";
  if(msg!=""){
    err = "<div style=\"font-weight:bold;margin-bottom: 20px; text-align: center;color:red\">\n\
             Error:&nbsp;&nbsp;<label style=\"font-weight:normal\">"+msg+"</label>\n\
           </div>";
  }
  return "<body>\n\
      <div style=\"padding: 10px 20px;margin-top:40px\">\n\
        <div class=\"loginWrapper\" style=\"margin-left: auto;margin-right: auto;\">\n\
            <div style=\"position: relative;margin: auto;width: 100%;\">\n\
                <div style=\"display: block;width:100%\"><h2>Log In</h2></div>\n\
                <div class=\"pinContainer\">\n"+err+"\
                    <form method=\"POST\" action=\"/login\">\n\
                        <input type=\"password\" name=\"PASSWORD\" placeholder=\"Enter Security Password\" autocomplete=\"off\">\n\
                        <input type=\"submit\" class=\"btn\" id=\"btnSubmit\" value=\"Log In\">\n\
                </form>\n\
                </div>\n\
            </div>\n\
        </div>\n\
      </div>\n\
      <br>";
}

// Full login page
String getLoginPage(String msg){
  return getHeadTitle("FYP: Login")+style+getLoginBody(msg)+end;
}

#endif
