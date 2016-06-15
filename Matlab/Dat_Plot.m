[FileName,PathName] = uigetfile('*.dat','Select a file');
file = fopen([PathName,FileName]);

fgetl(file);
val = textscan(file, '%f');
val = val{1:1};

fclose(file);

base = 0:1:length(val)-1;
figure('name',FileName);
plot(base, val)
abl = diff(val);
abl(find(abs(abl) >= 5000))=5000;
hold on
%plot(base(2:end), ((abl-1))+500)
%ylim([min(val), max(val)]);
