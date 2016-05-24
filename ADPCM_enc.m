% ADPCM Encoder
clear all
% read wave file
[a, Fs] = audioread('R.I.O. feat. U-Jean - Summer Jam.wav');

% 50ms sample of wave file
anz = Fs * 100;
pos = Fs * 10;  % Set cut offset (Cut of top of song)
x = a(pos:pos+anz-1, 1);    % left channel
%sound(x,Fs);

K = length(x);      % length of sample
N = 6;              % predictor order

% Initialization
e = zeros(N+1, K);  % predection error, all indicies are +1 because MatLab starts at 1 and not at 0
e(0+1,:) = x;         % e(0)(k) = x(k)

b = zeros(N+1, K);  % all indicies are +1
b(0+1,:) = x;         % b(0)(k) = x(k)

y = zeros(1, N-1 +1);    % prediction factor


% Burg Algorithm Nth order predictor:
for n = 1:N-1 +1
   
    num = 0;
    den = 0;
    
    for k = n:K-1   % numerator sum  
        num = num + e(n, k+1)*b(n, k);    
    end;
    
    for k = 1:K-1   % denominator sum
        den = den + e(n, k+1)^2 + b(n, k)^2;     
    end;
    
    y(n) = 2 * num / den;    % nth reflection factor
    
    e(n+1, n+1:K) = e(n, n+1:K) - y(n) * b(n, n:K-1);
    b(n+1, n+1:K) = b(n, n:K-1) - y(n) * e(n, n+1:K);
        
end;

for k = 1:N
    e(N+1, k) = e(k, k);
end;


clearvars -except y e N Fs

e = e(7,:);

ef = zeros(1,7);
bf = zeros(2,7);

% ADPCM decoder


for k = 1:length(e)
    ef(7) = e(k);  % Neuer Wert
    
for i = N:-1:1
   ef(i) = ef(i+1) + bf(1,i)*y(i);
   bf(1,i+1) = ef(i)*(-y(i))+bf(1,i);
end;
bf(1,1) = ef(1);
x(k) = ef(1);
end;

sound(x,Fs);

