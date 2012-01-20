/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */
"use strict";

const ZOOM = 2;
const RESIZE = 50;

function test() {
  let random = Math.random() * 10;

  TiltUtils.setDocumentZoom(window, random);
  ok(isApprox(TiltUtils.getDocumentZoom(window), random),
    "The getDocumentZoom utility function didn't return the expected results.");


  if (!isTiltEnabled()) {
    info("Skipping controller test because Tilt isn't enabled.");
    return;
  }
  if (!isWebGLSupported()) {
    info("Skipping controller test because WebGL isn't supported.");
    return;
  }

  waitForExplicitFinish();

  createTab(function() {
    createTilt({
      onInspectorOpen: function()
      {
        TiltUtils.setDocumentZoom(window, ZOOM);
      },
      onTiltOpen: function(instance)
      {
        ok(isApprox(instance.presenter.transforms.zoom, ZOOM),
          "The presenter transforms zoom wasn't initially set correctly.");

        let contentWindow = gBrowser.selectedBrowser.contentWindow;
        let initialWidth = contentWindow.innerWidth;
        let initialHeight = contentWindow.innerHeight;

        let renderer = instance.presenter.renderer;
        let arcball = instance.controller.arcball;

        ok(isApprox(contentWindow.innerWidth * ZOOM, renderer.width, 1),
          "The renderer width wasn't set correctly before the resize.");
        ok(isApprox(contentWindow.innerHeight * ZOOM, renderer.height, 1),
          "The renderer height wasn't set correctly before the resize.");

        ok(isApprox(contentWindow.innerWidth * ZOOM, arcball.width, 1),
          "The arcball width wasn't set correctly before the resize.");
        ok(isApprox(contentWindow.innerHeight * ZOOM, arcball.height, 1),
          "The arcball height wasn't set correctly before the resize.");


        window.resizeBy(-RESIZE * ZOOM, -RESIZE * ZOOM);

        executeSoon(function() {
          ok(isApprox(contentWindow.innerWidth + RESIZE, initialWidth, 1),
            "The content window width wasn't set correctly after the resize.");
          ok(isApprox(contentWindow.innerHeight + RESIZE, initialHeight, 1),
            "The content window height wasn't set correctly after the resize.");

          ok(isApprox(contentWindow.innerWidth * ZOOM, renderer.width, 1),
            "The renderer width wasn't set correctly after the resize.");
          ok(isApprox(contentWindow.innerHeight * ZOOM, renderer.height, 1),
            "The renderer height wasn't set correctly after the resize.");

          ok(isApprox(contentWindow.innerWidth * ZOOM, arcball.width, 1),
            "The arcball width wasn't set correctly after the resize.");
          ok(isApprox(contentWindow.innerHeight * ZOOM, arcball.height, 1),
            "The arcball height wasn't set correctly after the resize.");


          window.resizeBy(RESIZE * ZOOM, RESIZE * ZOOM);

          Services.obs.addObserver(cleanup, DESTROYED, false);
          InspectorUI.closeInspectorUI();
        });
      },
    });
  });
}

function cleanup() {
  Services.obs.removeObserver(cleanup, DESTROYED);
  gBrowser.removeCurrentTab();
  finish();
}
