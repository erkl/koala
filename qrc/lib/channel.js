var __bridge = window.__bridge;

var util = require('./util.js');


/* Map used for global channel lookup. */
var channels = {};


/* A Channel represents a namespaced stream of JSON messages received from
 * another process through stdin, and emitted over stdout. */
function Channel(name) {
  this.name = name;

  util.Emitter.call(this);
}

util.extend(Channel, util.Emitter);


/* Create a new channel with the given name, or re-use an existing one if
 * it already exists. */
Channel.open = function (name) {
  return channels[name] = channels[name] || new Channel(name);
};


/* Send a message over the channel. */
Channel.prototype.send = function (message) {
  var envelope = {};
  envelope[this.name] = message != null ? message : null;

  __bridge.messageSent(JSON.stringify(envelope));
};


/* Route incoming messages to their respective channels. */
__bridge.messageReceived.connect(function (raw) {
  var envelope;

  try {
    envelope = JSON.parse(raw);
  } catch (err) {
    return;
  }

  for (var key in envelope) {
    var chan = channels[key];
    if (chan != null)
      chan.emit('message', envelope[key]);
  }
});


module.exports = Channel;
