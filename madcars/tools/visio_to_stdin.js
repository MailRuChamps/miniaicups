// usage:     node visio_to_stdin.js [player] [visio]
// example 1: node visio_to_stdin.js 1 visio
// example 2: node visio_to_stdin.js 2 visio
// example 3: node visio_to_stdin.js 2 312719
//                                   ^ ^
//                                   | game_id
//                           your_side
var fs=require('fs');
var process=require('process');
var url=require("url");
var http=require("http");
var https=require("https");
var child_process=require('child_process');
var execSync=child_process.execSync;var exec=child_process.exec;

var log=s=>{if(1)process.stderr.write(s+"\n");};var qap_log=log;
if(process.argv.length<3)console.log("process.argv.length!=3");
var your_side=process.argv.length<3?0:process.argv[2];
var fn=process.argv.length<4?"visio":process.argv[3];

var qap_err=(context,err)=>context+" :: err = "+inspect(err)+" //"+err.stack.toString();
var log_err=(context,err)=>qap_log(qap_err(context,err));

var json=JSON.stringify;

var json_once=(obj,replacer,indent,limit)=>{
  var objs=[];var keys=[];if(typeof(limit)=='undefined')limit=2048;
  return json(obj,(key,v)=>{
    if(objs.length>limit)return 'object too long';
    var id=-1;objs.forEach((e,i)=>{if(e===v){id=i;}});
    if(key==''){objs.push(obj);keys.push("root");return v;}
    if(id>=0){
      return keys[id]=="root"?"(pointer to root)":
        ("\1(see "+((!!v&&!!v.constructor)?v.constructor.name.toLowerCase():typeof(v))+" with key "+json(keys[id])+")");
    }else{
      if(v!==null&&typeof(v)==="object"){var qk=key||"(empty key)";objs.push(v);keys.push(qk);}
      return replacer?replacer(key,v):v;
    }
  },indent);
};
var json_once_v2=(e,v,lim)=>json_once(e,v,2,lim);
var inspect=json_once_v2;

var call_cb_on_err=(emitter,cb,...args)=>{
  emitter.on('error',err=>{
    cb("'inspect({args,err}) // stack': "+inspect({args:args,err:err})+" // "+err.stack.toString());
  });
}

var xhr_get=(URL,ok,err)=>{
  if((typeof ok)!="function")ok=()=>{};
  if((typeof err)!="function")err=()=>{};
  var secure=['https:','https'].includes(url.parse(URL).protocol);
  var req=(secure?https:http).get(URL,(res)=>{
    var cb=ok;
    if(res.statusCode!==200){cb=(s,res)=>err('Request Failed.\nStatus Code: '+res.statusCode+'\n'+s);}
    //res.setEncoding('utf8');
    var rawData='';res.on('data',(chunk)=>rawData+=chunk.toString("binary"));
    res.on('end',()=>{try{cb(rawData,res);}catch(e){err(qap_err('xhr_get.mega_huge_error',e),res);}});
  });
  call_cb_on_err(req,qap_log,'xhr_get');
  return req;
}
var is_num=s=>parseInt(s)+""==s;

var main=fn=>
{
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
  process.stdout.on('close',()=>process.exit(0));
  process.stdin.on('close',()=>process.exit(0));
}

var zlib = require("zlib");
var drop_visio=e=>{var t=e.split("/visio");return t.length==2&&t[1].length==0?t[0]:"fail";}
var exec_sync_guard=e=>drop_visio(e).split("/").map(e=>!e.split(/[0123456789]+/).join("").length?"":"no_way").join("").length==0;
if(is_num(fn))
{
  var ptr="https://storage.aicups.ru/public/visio/";
  var dref='data-href="'+ptr;
  var cb=s=>{
    var p=s.split(dref)[1].split('"')[0];
    if(!exec_sync_guard(p))throw new Error("wrong response. p="+p);
    //xhr_get(ptr+p,s=>{
    fn+=".visio";
    var fngz=fn+".gz";
    if(!fs.existsSync(fn)){
      execSync("curl "+ptr+p+">"+fngz);
      
      //execSync("gunzip --version -d "+fngz);
      var gunzip=zlib.createGunzip();
      var out=fs.createWriteStream(fn);
      fs.createReadStream(fngz).pipe(gunzip).pipe(out);
      out.on('close',()=>main(fn));
      //out.on('end',()=>main(fn));
    }else{main(fn);}
    //fs.writeFileSync(fn,s.toString("binary"));
    // main(fn);
    //},log);
  }
  xhr_get("https://aicups.ru/session/"+fn+"/?a=get_viso_url",cb,log);
}
