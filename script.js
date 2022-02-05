$('#fileupload input[type=submit]').click(function() {
  $.ajax({
      url: '/',
      type: 'POST',
      data: new FormData(document.getElementById('fileupload')),
      success: function(res) {
          alert(res);
      }               
  });
});