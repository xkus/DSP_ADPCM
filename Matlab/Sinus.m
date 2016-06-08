fs = 44100;
f = 440;
t = 0:1/fs:1;
sig = sin(2*pi*f*t);
plot(t,sig)
%sound(sig,fs)
samp2 = round( sig*(2^10 / 2));

file = fopen(['Sinus','.txt'], 'w');
nzeile = 50; % Werte pro Zeile
outp_i = 0;

fprintf(file, 'short MySound[%i] = {\n', length(samp2));
for i = 0:(length(samp2)/nzeile)-1
    
    fprintf(file, '%i,', samp2(i*nzeile+1:(i*nzeile+nzeile)) );
    fprintf(file, '\n');

end;

fprintf(file, '};\n');
soundsc(double(samp2),fs)
length(samp2)