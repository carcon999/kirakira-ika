let hue = 0
let sat = 100

const colorChange = () => {
  document.querySelector('.colorname').textContent = "色調:" + hue + " 彩度:" + sat + "％";
  const swatch = document.querySelector('.swatch')
  swatch.style.backgroundColor = getHSL()
  document.querySelector('.satcolor').style.backgroundColor = getHSL()
  
  var rgb = hslToRgb(hue/360, sat/100, sys_bright/511);
  ws_request_color(rgb);
}

const getHSL = () => {
  return `hsl(${hue}, ${sat}%, ${sys_bright*100/128}%)`
}

const main = () => {
  const hueInput = document.querySelector('input[name=hue]')
  hueInput.addEventListener('input', () => {
    hue = Number(hueInput.value)
    console.log(getHSL())
    colorChange()
  })

  const satInput = document.querySelector('input[name=sat]')
  satInput.addEventListener('input', () => {
    sat = Number(satInput.value)
    console.log(getHSL())
    colorChange()
  })

//  const lightInput = document.querySelector('input[name=light]')
//  lightInput.addEventListener('input', () => {
//    light = Number(lightInput.value)
//    console.log(getHSL())
//    colorChange()
//  })

//  colorChange()
}

document.addEventListener('DOMContentLoaded', main)

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   {number}  h       The hue
 * @param   {number}  s       The saturation
 * @param   {number}  l       The lightness
 * @return  {Array}           The RGB representation
 */
function hslToRgb(h, s, l){
    var r, g, b;

    if(s == 0){
        r = g = b = l; // achromatic
    }else{
        var hue2rgb = function hue2rgb(p, q, t){
            if(t < 0) t += 1;
            if(t > 1) t -= 1;
            if(t < 1/6) return p + (q - p) * 6 * t;
            if(t < 1/2) return q;
            if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
            return p;
        }

        var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        var p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3);
    }

    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}
