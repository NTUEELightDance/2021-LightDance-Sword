const { strict } = require("assert");
const fs = require("fs"),
  PNG = require("pngjs").PNG;





for (let frame = 0; frame < 7; frame += 1){
  const png = new PNG({
    width: 2,
    height: 30,
  });
  let outputFIleName = "./output/red_sword_shadow_"+String(7-frame)+".png";
  for (let y = 0; y < png.height; y += 1) {
    for (let x = 0; x < png.width; x += 1) {
      if(y >= 4 * frame && y < 4 * (frame+1)) {
        let idx = (png.width * y + x) << 2;
        png.data[idx] = 255; // red
        png.data[idx + 1] = 0; // green
        png.data[idx + 2] = 0; // blue
        png.data[idx + 3] = 128; // alpha (0 is transparent)
      }
      else {
        let idx = (png.width * y + x) << 2;
        png.data[idx] = 0; // red
        png.data[idx + 1] = 0; // green
        png.data[idx + 2] = 0; // blue
        png.data[idx + 3] = 128; // alpha (0 is transparent)
      }
    }
  }
  png.pack().pipe(fs.createWriteStream(outputFIleName));
}

