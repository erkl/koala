var __bridge = window.__bridge;

var util = require('./util.js');


/* QWebFrame JavaScript handles and Frame instances. */
var handles = [];
var frames = [];


/* The Frame "class" represents a frame on the web page. */
function Frame(parent, ref) {
  this.parent = parent;
  this.children = [];

  this.element = ref.frameElement;
  this.window = ref.window;
  this.document = ref.document;

  this.__intercepts = {};

  util.Emitter.call(this);
}

util.extend(Frame, util.Emitter);


/* Creates a new frame. */
Frame.create = function (url, size) {
  /* Between these two lines, we'll catch the 'frameSpawned' signal
   * emitted for the iframe we've just created. */
  document.body.appendChild(document.createElement('iframe'));
  return frames[frames.length - 1];
};


/* Navigate to a URL. */
Frame.prototype.go = function (url) {
  if (this.element != null)
    this.element.src = url;
};


/* Register an intercept function. */
Frame.prototype.intercept = function (name, fn) {
  this.__intercepts[name] = fn;
};


/* Create a Frame instance for every frame inserted into the page. */
__bridge.frameSpawned.connect(function (document, handle, parentHandle) {
  var parent = frames[handles.indexOf(parentHandle)] || null;
  var frame = null;

  /* Iterate through `window.frames` in search of the Frame instance associated
   * with the frame that was just created. */
  for (var i = 0; i < (parent ? parent.window : window).frames.length; i++) {
    var ref = (parent ? parent.window : window).frames[i];
    if (ref.document === document.parentNode) {
      frame = new Frame(parent, ref);
      break;
    }
  }

  /* Bail if we couldn't find the frame. This shouldn't happen. */
  if (frame == null)
    return;

  /* Attach listeners for simple signals. */
  handle.loadStarted.connect(function () { frame.emit('loading'); });
  handle.loadFinished.connect(function () { frame.emit('loaded'); });
  handle.initialLayoutCompleted.connect(function () { frame.emit('layout'); });

  handle.javaScriptWindowObjectCleared.connect(function () {
    /* Update outdated properties. */
    frame.children = [];
    frame.window = ref.window;
    frame.document = ref.document;

    frame.emit('cleared');
  });

  /* By monitoring the QWebFrame's 'destroyed' signal, we're able to detect
   * when the frame is removed from the DOM. */
  handle.destroyed.connect(function () {
    frame.parent = null;
    frame.children = [];

    frame.element = null;
    frame.window = null;
    frame.document = null;

    /* Zero out relevant indexes in `frames` and `handles`. */
    var i = frames.indexOf(frame);
    if (i >= 0) {
      frames[i] = null;
      handles[i] = null;
    }

    /* Make this frame unreachable from the parent frame. */
    frame.parent.children = frame.parent.children.filter(function (child) {
      return child !== frame;
    });

    /* Finally, notify the user. */
    frame.emit('destroyed');
  });

  /* Register the handle and the Frame instance. */
  handles.push(handle);
  frames.push(frame);

  /* Emit a 'child' event on the parent. */
  if (parent != null) {
    parent.children.push(frame);
    parent.emit('child', frame);
  }
});


/* Listen for callback requests. */
__bridge.callbackRequested.connect(function (name, handle, args) {
  __bridge.setCallbackValue(null);

  /* Invoke the frame's relevant intercept function. */
  var frame = frames[handles.indexOf(handle)] || null;

  if (frame != null && frame.__intercepts[name] != null) {
    var ret = frame.__intercepts[name].apply(frame, args);
    __bridge.setCallbackValue(ret);
  }
});


module.exports = Frame;
