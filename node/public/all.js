$(function() {
  var socket = io.connect();

  // update background of page with color from server
  socket.on('color', function (color) {
    $('.ui-page').css('background', color );
  });

  // when a slider changes
  $('#red, #green, #blue').on('change', function(){
    var colors = {r: Number($('#red').val()), g: Number($('#green').val()), b: Number($('#blue').val())};
    socket.emit('color', colors);
  });

  // change mode
  $('#mode a').on('click', function(){
    socket.emit('mode', $(this).text().toLowerCase());
  });
});
