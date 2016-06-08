[FileName,PathName] = uigetfile('*.dat','Select a file');
file = fopen(FileName);

fgetl(file);
val = textscan(file, '%f');

fclose(file);

base = 1:1:length(val{1:1});

plot(base, val{1:1})
