% Buffer plot on schön

[FileName,PathName] = uigetfile('*.dat','Select a file');
file = fopen([PathName,FileName]);
write_i = input('\n\nWrite_i eingeben:\n'); % Offset in Sekunden
fgetl(file);
bin = textscan(file, '%f');
bin = bin{1:1};

fclose(file);
val = [bin(write_i+1:end)' bin(1:write_i)'];
val2 = bin(bin ~= 32767);
val3 = val2(val2 ~= -32768);




figure('name',FileName);

plot(val)
abl = diff(val);
hold on
plot(base(2:end), abl)

ylim([min(val), max(val)]);

figure('name',FileName);
subplot(2,1,1)
plot(bin)
hold on
abl2 = diff(bin);
abl2(find(abs(abl2) >= 5000))=5000;
%ylim([min(bin), max(bin)]);
plot(abl2)
subplot(2,1,2)
plot(val3)