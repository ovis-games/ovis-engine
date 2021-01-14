let Module = {
  preRun: [],
  postRun: [],
  print: (function () {
    return function (text: string) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      // These replacements are necessary if you render to raw HTML
      //text = text.replace(/&/g, "&amp;");
      //text = text.replace(/</g, "&lt;");
      //text = text.replace(/>/g, "&gt;");
      //text = text.replace('\n', '<br>', 'g');
      console.log(text);
    };
  })(),
  printErr: function (text: string) {
    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
    console.error(text);
  },
  onRuntimeInitialized: function() {
    console.log("Runtime initialized!");
  },
  arguments: ["", "x", "2"],
  canvas: (function () {
    var canvas = document.getElementById('canvas');

    // As a default initial behavior, pop up an alert when webgl context is lost. To make your
    // application robust, you may want to override this behavior before shipping!
    // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
    canvas?.addEventListener("webglcontextlost", function (e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

    canvas?.addEventListener("dragenter", function (event) {
      event.preventDefault();
      event.stopPropagation();
      Module['DragEnter'](event.dataTransfer);
    });

    canvas?.addEventListener("dragover", function (event) {
      event.preventDefault();
      event.stopPropagation();
      Module['DragOver'](event);
    });

    canvas?.addEventListener("dragleave", function (event) {
      event.preventDefault();
      event.stopPropagation();
      Module['DragLeave'](event.dataTransfer);

    });

    canvas?.addEventListener("drop", function (event) {
      event.preventDefault();
      event.stopPropagation();
      Module['Drop'](event.dataTransfer);
    });

    return canvas;
  })(),
  setStatus: function (text: string) {
    console.log(text);
  },
  totalDependencies: 0,
  monitorRunDependencies: function (left: number) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    console.log(left ? 'Preparing... (' + (this.totalDependencies - left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
  },
  locateFile: (path: string, prefix: string) => {
    console.log("Prefix: " + prefix);
    console.log("path: " + path);
    return (prefix || `editor_versions/${this.version}/`) + path;
  }
};