// usage:     node visio_to_stdin.js [player] [visio]
// example 1: node visio_to_stdin.js 1 visio
// example 2: node visio_to_stdin.js 2 visio
var fs=require('fs');
var process=require('process');
var log=s=>{if(1)process.stderr.write(s+"\n");}
if(process.argv.length<3)console.log("process.argv.length!=3");
var your_side=process.argv.length<3?0:process.argv[2];
var fn=process.argv.length<4?"visio":process.argv[3];
var a=your_side;var b=your_side==1?2:1;
log(JSON.stringify({your_side:your_side,fn:fn,me:a,enemy:b}));
var s=fs.readFileSync(fn)+"";
var obj=JSON.parse(s);
var arr=obj.visio_info;
var jp=e=>JSON.parse(e);
var both_alive=e=>e.my_car.length==6?jp(e.my_car[5])&&jp(e.enemy_car[5]):true;
arr=arr.map((e,i)=>{
  var p=e.params;
  //if(i>10){e.params=[];return;}
  if(e.type=="tick"){
    var t={"my_car":p.cars[a],"enemy_car":p.cars[b],deadline_position:p.deadline_position};
    e.params=t;
    return e;
  }
  if(e.type=="new_match"){
    var L=p.lives;
    p.my_lives=L[a];
    p.enemy_lives=L[b];
  }
  return e;
}).filter(e=>e.type!="tick"||both_alive(e.params));
var out=arr.map(e=>JSON.stringify(e)).join("\n");//JSON.stringify(arr,0,2);
//fs.writeFileSync("pretty."+fn,out);
process.stdout.write(out);
