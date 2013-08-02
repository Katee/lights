$(function() {
  var socket = io.connect();
  var colorSliders = [$('#red'), $('#green'), $('#blue')];

  // update background of page with color from server
  socket.on('color', function (color) {
    $('.ui-page').css('background', color );
    var colors = color.split(',').map(function(string){return Number(string.replace(/[^0-9]/g, ''));});
    colors.zip(colorSliders).each(function(thing){
      $(thing[1]).val(thing[0]).slider('refresh');
    });
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
