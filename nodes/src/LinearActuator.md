# Linear Actuator State Variables


pos: >0, 0, <0

pcnt
last_pcnt
last_pcnt_micros


motor_speed: <0, 0, >0
limit: 0, 1
target: >0, 0
err = target_position - current_position: <0, 0, >0


This looks like a problem when the lock jams
Jun 16 08:09:16 yarrow python3[67281]: 08:09:16.107 Thread-2: Opening
Jun 16 08:09:16 yarrow python3[67281]: 08:09:16.304 MainThread: Rx from:2, msg:gp:0,acp:700,ap:700,at:0,al:0,ms:0,aloe:0,asf:0
Jun 16 08:09:16 yarrow python3[67281]: 08:09:16.583 MainThread: Rx from:2, msg:gp:0,acp:700,ap:695,at:0,al:0,ms:-96,aloe:0,asf:200
Jun 16 08:09:16 yarrow python3[67281]: 08:09:16.850 MainThread: Rx from:2, msg:gp:1,acp:700,ap:692,at:0,al:0,ms:-96,aloe:0,asf:200
Jun 16 08:09:17 yarrow python3[67281]: 08:09:17.118 MainThread: Rx from:2, msg:gp:1,acp:700,ap:687,at:0,al:0,ms:-100,aloe:0,asf:200
Jun 16 08:09:17 yarrow python3[67281]: 08:09:17.376 MainThread: Rx from:2, msg:gp:2,acp:700,ap:680,at:0,al:0,ms:-133,aloe:0,asf:200
Jun 16 08:09:17 yarrow python3[67281]: 08:09:17.643 MainThread: Rx from:2, msg:gp:100,acp:700,ap:-1,at:0,al:0,ms:-166,aloe:672,asf:201
Jun 16 08:09:17 yarrow python3[67281]: 08:09:17.911 MainThread: Rx from:2, msg:gp:101,acp:700,ap:-12,at:0,al:0,ms:-192,aloe:672,asf:201
Jun 16 08:09:18 yarrow python3[67281]: 08:09:18.179 MainThread: Rx from:2, msg:gp:103,acp:700,ap:-23,at:0,al:0,ms:-192,aloe:672,asf:201
Jun 16 08:09:18 yarrow python3[67281]: 08:09:18.437 MainThread: Rx from:2, msg:gp:105,acp:700,ap:-37,at:0,al:0,ms:-192,aloe:672,asf:201



current state                        | output
speed | limit | target | err  | speed | target | pos
<0         0       >0     >0   |  err      -       -
<0         0       >0      0   |    0      -       -
<0         0       >0     <0   |  err      -       -
<0         0        0     >0   |  -64      -       -
<0         0        0      0   |  -64      -       -
<0         0        0     <0   |  err      -       -
<0         0       <0     >0   |  -64      -       -
<0         0       <0      0   |  -64      -       -
<0         0       <0     <0   |  err      -       -

<0         1       >0     >0   |  err      -       -
<0         1       >0      0   |    0      0       0
<0         1       >0     <0   |    0      0       0
<0         1        0     >0   |    0      0       0
<0         1        0      0   |    0      0       0
<0         1        0     <0   |    0      0       0
<0         1       <0     >0   |    0      0       0
<0         1       <0      0   |    0      0       0
<0         1       <0     <0   |    0      0       0

0         0       >0     >0   |  err      -       -
0         0       >0      0   |    0      -       -
0         0       >0     <0   |  err      -       -
0         0        0     >0   |  err      -       -
0         0        0      0   |  -64      -       -
0         0        0     <0   |  -64      -       -
0         0       <0     >0   |  -64      -       -
0         0       <0      0   |  -64      -       -
0         0       <0     <0   |  err      -       -

0         1       >0     >0   |  err      -       -
0         1       >0      0   |    0      0       0
0         1       >0     <0   |    0      0       0
0         1        0     >0   |    0      0       0
0         1        0      0   |    0      0       0
0         1        0     <0   |    0      0       0
0         1       <0     >0   |    0      0       0
0         1       <0      0   |    0      0       0
0         1       <0     <0   |    0      0       0

>0         0       >0     >0   |  err      -       -
>0         0       >0      0   |  err      -       -
>0         0       >0     <0   |  err      -       -
>0         0        0     >0   |  -64      -       -
>0         0        0      0   |  -64      -       -
>0         0        0     <0   |  err      -       -
>0         0       <0     >0   |  -64      -       -
>0         0       <0      0   |  -64      -       -
>0         0       <0     <0   |  err      -       -

>0         1       >0     >0   |  err      -       -
>0         1       >0      0   |    0      -       -
>0         1       >0     <0   |    0      0       0
>0         1        0     >0   |    0      0       0
>0         1        0      0   |    0      0       0
>0         1        0     <0   |    0      0       0
>0         1       <0     >0   |    0      0       0
>0         1       <0      0   |    0      0       0
>0         1       <0     <0   |    0      0       0

=====================================================


b0=speed==0
b1=speed>0




if (limit)
{
  if ( tp > 0 && err > 0)
  {
0         1       >0     >0   |  err      -       -
<0        1       >0     >0   |  err      -       -
>0        1       >0     >0   |  err      -       -
  }
  else if (tp > 0 && err == 0)
  {
>0        1       >0      0   |    0      -       -
  }
  else
  {
<0        1       >0      0   |    0      0       0
<0        1       >0     <0   |    0      0       0
<0        1        0     >0   |    0      0       0
<0        1        0      0   |    0      0       0
<0        1        0     <0   |    0      0       0
<0        1       <0     >0   |    0      0       0
<0        1       <0      0   |    0      0       0
<0        1       <0     <0   |    0      0       0
0         1       >0      0   |    0      0       0
0         1       >0     <0   |    0      0       0
0         1        0     >0   |    0      0       0
0         1        0      0   |    0      0       0
0         1        0     <0   |    0      0       0
0         1       <0     >0   |    0      0       0
0         1       <0      0   |    0      0       0
0         1       <0     <0   |    0      0       0
>0        1       >0     <0   |    0      0       0
>0        1        0     >0   |    0      0       0
>0        1        0      0   |    0      0       0
>0        1        0     <0   |    0      0       0
>0        1       <0     >0   |    0      0       0
>0        1       <0      0   |    0      0       0
>0        1       <0     <0   |    0      0       0
  }
}
else (limit==0)
{
  if (tp>0)

<0        0       >0     >0   |  err      -       -
<0        0       >0      0   |  err      -       -
<0        0       >0     <0   |  err      -       -
0         0       >0     >0   |  err      -       -
0         0       >0      0   |  err      -       -
0         0       >0     <0   |  err      -       -
>0        0       >0     >0   |  err      -       -
>0        0       >0      0   |  err      -       -
>0        0       >0     <0   |  err      -       -
  else if (tp == 0)
  {
    if ((speed < 0  && err < 0) ||
        (speed == 0 && err > 0) ||
        (speed > 0  && err < 0)
    {
<0        0        0     <0   |  err      -       -
0         0        0     >0   |  err      -       -
>0        0        0     <0   |  err      -       -
    }
    else
    {
<0        0        0     >0   |  -64      -       -
<0        0        0      0   |  -64      -       -
0         0        0      0   |  -64      -       -
0         0        0     <0   |  -64      -       -
>0        0        0     >0   |  -64      -       -
>0        0        0      0   |  -64      -       -
    }
  }
  else
  {
    if (err < 0)
    {
>0        0       <0     <0   |  err      -       -
<0        0       <0     <0   |  err      -       -
0         0       <0     <0   |  err      -       -
    }
    else
    {
<0        0       <0     >0   |  -64      -       -
<0        0       <0      0   |  -64      -       -
0         0       <0     >0   |  -64      -       -
0         0       <0      0   |  -64      -       -
>0        0       <0     >0   |  -64      -       -
>0        0       <0      0   |  -64      -       -
    }
  }
}