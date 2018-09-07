// usage:   node visio_to_stdin.js [player] [visio]
// example: node visio_to_stdin.js 0 visio
// example: node visio_to_stdin.js 1 visio
var fs=require('fs');
var process=require('process');
if(process.argv.length<3)console.log("process.argv.length!=3");
var your_side=process.argv.length<3?0:process.argv[3];
var fn=process.argv.length<4?"visio":process.argv[4];
var s=fs.readFileSync(fn)+"";
var obj=JSON.parse(s);
var arr=obj.visio_info;
var a=your_side;var b=(a+1)%2;a++;b++;
arr.map((e,i)=>{
  var p=e.params;
  //if(i>10){e.params=[];return;}
  if(e.type=="tick"){
    var t={"my_car":p.cars[a],"enemy_car":p.cars[b]};
    e.params=t;
    return;
  }
  if(e.type=="new_match"){
    var L=p.lives;
    p.my_lives=L[a];
    p.enemy_lives=L[b];
  }
});
var out=arr.map(e=>JSON.stringify(e)).join("\n");//JSON.stringify(arr,0,2);
//fs.writeFileSync("pretty."+fn,out);
process.stdout.write(out);
