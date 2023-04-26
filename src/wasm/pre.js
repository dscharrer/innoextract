var Module = {
    'print': function(text) { console.log(text); con.insertAdjacentHTML('beforeend', '<p class="info">'+text+'</p>'); con.scrollTop = con.scrollHeight;},
    'printErr': function(text) { console.error(text); con.insertAdjacentHTML('beforeend', '<p class="err">'+text+'</p>'); con.scrollTop = con.scrollHeight;}
  };