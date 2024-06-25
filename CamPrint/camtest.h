#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"raw(
<!DOCTYPE html>
<html lang="en">

<body style="display: flex;flex-direction: column;align-items: center;">
  <h1>Camera test</h1>
  <div><button onclick="getFrame()">getFrame</button><button onclick="printFrame()">printFrame</button></div><br>
  <div style="max-width:800px"><canvas style="width:100%" id="canvas" width="640" height="480"></canvas></div>

  <script>
    async function getFrame() {
      let res = await fetch(window.location.origin + '/getFrame');
      let arr = await res.arrayBuffer();
      let data = new Uint8Array(arr);

      let cx = canvas.getContext("2d");
      let imageData = cx.createImageData(canvas.width, canvas.height);
      let pixels = imageData.data;

      data.forEach((x, i) => {
        pixels[i * 4 + 0] = x;
        pixels[i * 4 + 1] = x;
        pixels[i * 4 + 2] = x;
        pixels[i * 4 + 3] = 255;
      });
      cx.putImageData(imageData, 0, 0);

      resize();
    }

    function resize() {
      canvas.style.height = canvas.parentNode.clientWidth * canvas.height / canvas.width + 'px';
    }

    function printFrame() {
      fetch(window.location.origin + '/printFrame');
    }

    window.addEventListener('resize', () => resize());

    resize();
  </script>
</body>

</html>
)raw";