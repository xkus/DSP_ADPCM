[FileName,PathName] = uigetfile('*.dat','Select a file');
file = fopen([PathName,FileName]);

fgetl(file);
val = textscan(file, '%f');
val = val{1:1};

fclose(file);

base = 1:1:length(val);
figure('name',FileName);
plot(base, val)
abl = diff(val);
hold on
%plot(base(2:end), ((abl-1)*200)-(min(val)- max(val))/2)
ylim([min(val), max(val)]);
