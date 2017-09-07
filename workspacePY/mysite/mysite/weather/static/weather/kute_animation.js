var el1 = document.querySelector('.box1');
var el2 = document.querySelector('.box2');
var el3 = document.querySelector('.box3');
var el4 = document.querySelector('.box4');
var el5 = document.querySelector('.Learn_button');
var el6 = document.querySelector('.telegram_icon');



// built the tween objects
var tween1 = KUTE.fromTo(el1,{rotate:0},{rotate:360},{duration:1500});
var tween2 = KUTE.to(el2,{translate3d:[0,0,10]},{repeat:1000,perspective:100,duration:500});
var tween3 = KUTE.fromTo(el3,{rotateY:0},{rotateY:360}, {duration: 1500} );
var tween4 = KUTE.fromTo(el4,{rotateZ:0},{rotateZ:-360},{duration: 1500});
var tween5 = KUTE.to(el5,{translate3d:[0,0,15]},{repeat:10000,perspective:100,duration:2000,yoyo: true});
var tween6 = KUTE.to(el6,{translate3d:[0,0,15]},{repeat:10000,perspective:100,duration:2000,yoyo: true});


tween2.start(); // for ticks
tween5.start();
tween6.start();



setInterval(iLoveOpenData,8000);


function iLoveOpenData(){
tween1.start();
tween3.start();
tween4.start();
} 
