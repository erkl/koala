var __bridge = window.__bridge;

var Channel = require('./channel.js');
var Frame = require('./frame.js');
var util = require('./util.js');


/* Create our `koala` handle. */
var koala = new util.Emitter();


/* Create a new frame and navigate to the specified URL. */
koala.open = function (url, options) {
  var frame = Frame.create();

  /* Set the iframe size. */
  frame.element.width = (options && options.width) || 1280;
  frame.element.height = (options && options.height) || 720;

  /* Wait a tick to give the user a chance to attach listeners and custom
   * hooks before events start firing. */
  setTimeout(frame.go.bind(frame, url), 0);

  return frame;
};


/* Create a new namespaced channel of communication. */
koala.channel = function (name) {
  return Channel.open('' + name);
};


/* We store the list of current cookies as a JSON-encoded list to make sure
 * the user can't accidentally modify the array and bring it out of sync with
 * the actual cookie jar. */
var cookieList = '[]';


/* Return the list of cookies currently in the network stack's cookie jar,
 * or overwrite it with a new list. */
koala.cookies = function (cookies) {
  if (arguments.length === 0)
    return JSON.parse(cookieList);
 else
    __bridge.setCookies(cookies);
}


/* Listen for changes to the cookie jar. */
__bridge.cookiesChanged.connect(function (cookies) {
  cookieList = JSON.stringify(cookies);
  koala.emit('cookies', koala.cookies());
});


/* Kill the koala process. */
koala.exit = function (code) {
  __bridge.exit(code | 0);
};


module.exports = koala;
