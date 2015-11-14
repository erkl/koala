var __bridge = window.__bridge;


/* Simple event emitter implementation. */
function Emitter() {
  this.__listeners = {};
}


/* Attach a listener. */
Emitter.prototype.on = function (event, func) {
  add.call(this, event, func, false);
};


/* Attach a listener which will only trigger once. */
Emitter.prototype.once = function (event, func) {
  add.call(this, event, func, true);
};


function add(event, func, once) {
  var list = this.__listeners[event] || [];
  if (list.push({ func: func, once: once }) === 1)
    this.__listeners[event] = list;
}


/* Remove a specific listener, or all listeners for a particular event if
 * `func` isn't defined. */
Emitter.prototype.off = function (event, func) {
  this.__listeners[event] = (this.__listeners[event] || []).filter(function (item) {
    return func != null && item.func !== func;
  });
};


/* Trigger an event. */
Emitter.prototype.emit = function (event) {
  var args = Array.prototype.slice.call(arguments, 1);
  var list = this.__listeners[event] || [];

  for (var i = 0; i < list.length; i++) {
    var func = list[i].func;

    /* Remove the listener if it's only meant to trigger once. */
    if (list[i].once)
      list.splice(i--, 1);

    try { func.apply(this, args); } catch (_) { }
  }

  /* Emit a wildcard event. */
  if (event !== '*')
    this.emit.apply(this, ['*', event].concat(args));
};


/* Make `child` a prototypical "subclass" of `parent`. */
function extend(child, parent) {
  child.prototype = Object.create(parent.prototype, {
    constructor: {
      value: child,
      enumerable: false,
      writable: true,
      configurable: true
    }
  });
}


module.exports = {
  Emitter: Emitter,
  extend: extend
};
