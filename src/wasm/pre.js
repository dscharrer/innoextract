var Module = {
    'print': function(text) { console.log(text); let con = document.getElementById("con"); con.innerHTML+='<p class="info">'+text+"</p>"; con.scrollTop = con.scrollHeight;},
    'printErr': function(text) { console.error(text); let con = document.getElementById("con"); con.innerHTML+='<p class="err">'+text+"</p>"; con.scrollTop = con.scrollHeight;}
  };