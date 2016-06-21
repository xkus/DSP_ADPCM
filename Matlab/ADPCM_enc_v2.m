% ADPCM Encoder
clear all
% read wave file
[FileName,PathName] = uigetfile('*','Select sound file');
[a, Fs] = audioread([PathName FileName], 'native');

% 50ms sample of wave file
anz = 10;% Fs * 0.05;
pos = Fs * 50;  % Set cut offset (Cut of top of song)
x = a(pos:pos+anz-1, 1);    % left channel
%sound(x,Fs);

% Create any signal
	%i = 0:1:anz-1;
	%x(i+1) = cos(i * 0.01) + 0.75 *cos(i * 0.03)+ 0.5 *cos(i * 0.05) + 0.25 *cos(i * 0.11);
   

%% Encode vector x
K = length(x);      % length of sample
N = 6;              % predictor order

% Initialization
e = zeros(N+1, K);  % prediction error, all indicies are +1 because MatLab starts at 1 and not at 0
e(0+1,:) = x;         % e(0)(k) = x(k)

b = zeros(N+1, K);  % all indicies are +1
b(0+1,:) = x;         % b(0)(k) = x(k)

y = zeros(1, N+1);    % prediction factor
y(0+1) = -1;
max_num = 0;
min_num = 0;

% Burg Algorithm Nth order predictor:
for n = 1:N-1 +1
   
    num = 0;
    den = 0;
    
    for k = n:K-1   % numerator sum  
        num = num + e(n, k+1)*b(n, k-1+1);    
    
    end;
    num
    if num > max_num
        max_num = num;
    end
    
    if num < min_num
        min_num = num;
    end
    
    for k = 1:K-1   % denominator sum
        den = den + e(n, k+1)^2 + b(n, k-1+1)^2;     
    end;
    den
    y(n+1) = 2 * num / den;    % nth reflection factor
    
    e(n+1, n+1:K) = e(n, n+1:K) - y(n+1) * b(n, n:K-1);
    b(n+1, n+1:K) = b(n, n:K-1) - y(n+1) * e(n, n+1:K);
        
end;

for k = 1:N
    e(N+1, k) = e(k, k);
end;
e = e(7,:);
v1_y = y;
v1_e = e;
%clearvars -except v1_y v1_e x

%input('\n\nWrite_i eingeben:\n')
%% decode
%clearvars -except y e N Fs x


N = length(v1_y)-1;
K = length(e);
ef = zeros(N+1,K);
bf = zeros(N+1,K);

ef(0+1,0+1) = e(0+1);
bf(0+1,0+1) = e(0+1);

% ADPCM decoder

for k = 1:K-1
    
    if k > N
        ef(N+1,k+1) = e(k+1);
    else
        ef(k+1,k+1) = e(k+1);
    end
    
    for n= min(k, N):-1:1
        
        ef(n,k +1) = ef(n +1,k +1) + y(n +1)*bf(n-1 +1,k-1 +1);
        bf(n +1, k +1) = bf(n-1 +1, k-1 +1) - y(n +1)*ef(n-1 +1, k +1);   
    end
    bf(0 +1,k +1)=ef(0 +1,k +1);
end


x1 = e(0+1,:);

%sound(x1,Fs);

