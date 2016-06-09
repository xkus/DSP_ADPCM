fs = 48000;
f = 440;
t = 0:1/fs:1;
sig = sin(2*pi*f*t);
plot(t,sig)
%sound(sig,fs)
samp2 = round( sig*(2^10 / 2));
samp2 = samp2(1:109);
file = fopen(['Sinus','.txt'], 'w');
nzeile = 50; % Werte pro Zeile
outp_i = 0;

fprintf(file, 'short MySound[%i] = {\n', length(samp2));
%for i = 0:(length(samp2)/nzeile)
    
    fprintf(file, '%i,', samp2);
    fprintf(file, '\n');

%end;

fprintf(file, '};\n');
%soundsc(double(samp2),fs)
length(samp2)

