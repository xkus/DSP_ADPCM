[FileName,PathName] = uigetfile('*.dat','Select a file');
file = fopen([PathName,FileName]);

fgetl(file);
val = textscan(file, '%f');

fclose(file);

base = 1:1:length(val{1:1});

plot(base, val{1:1})
abl = diff(val{1:1});
hold on
plot(base(2:end), abl*100)
ylim([min(val{1:1}), max(val{1:1})]);