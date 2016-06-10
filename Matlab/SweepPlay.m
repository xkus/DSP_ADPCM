Fs = 48000;
f = 500;
len = 12000/Fs;
t = 0:1/Fs:len;
t2 = len:-1/Fs:0;

sig = sin(2*pi*f*t).*t/len;

%sig = sin(2*pi*f*t);

sig2 = sin(2*pi*f*t2).*t2/len;

plot(t,sig)
    player = audioplayer(sig,Fs,16,6)
    player2 = audioplayer(sig2,Fs,16,6)
while i ~= 100;
 
 %pause(0.2);
 playblocking(player);
 pause(12500/Fs);
 %playblocking(player2);
 i = 0;
end