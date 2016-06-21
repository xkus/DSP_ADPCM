% ADPCM Encoder

% read wave file
[a, Fs] = audioread('R.I.O. feat. U-Jean - Summer Jam.wav');

% 50ms sample of wave file
anz = Fs * 0.05;
pos = 900000;
x = a(pos:pos+anz-1, 1);    % left channel

K = length(x);      % length of sample
N = 6;              % predictor order

% Initialization
%e = zeros(N+1, K);  % predection errror, all indicies are +1 because MatLab starts at 1 at not at 0
%e(1,:) = x;         % e(0)(k) = x(k)
e_n = x;
e_n1 = zeros(K);

%b = zeros(N+1, K);  % all indicies are +1
%b(1,:) = x;         % b(0)(k) = x(k)
b_n = x;
b_n1 = zeros(K);

y = zeros(1, N);    % prediction factor


% Burg Algorithm Nth order predictor:
for n = 1:N
   
    num = 0;
    den = 0;
    
    e_n1 = zeros(K);
    b_n1 = zeros(K);
    
    for k = n:K-1   % numerator sum  
        %num = num + e(n, k+1)*b(n, k);
        num = num + e_n(k+1)*b_n(k);
    end;
    
    for k = 1:K-1   % denominator sum
        %den = den + e(n, k+1)^2 + b(n, k)^2;
        den = den + e_n(k+1)^2 + b(k)^2;
    end;
    
    y(n) = 2 * num / den;    % nth reflection factor
    
    %e(n+1, n+1:K) = e(n, n+1:K) - y(n) * b(n, n:K-1);
    e_n1(n+1:K) = e_n(n+1:K) - y(n) * b_n(n:K-1);
    %b(n+1, n+1:K) = b(n, n:K-1) - y(n) * e(n, n+1:K);
    b_n1(n+1:K) = b_n(n:K-1) - y(n) * e_n(n+1:K);
    
    e_n = e_n1;

    b_n = b_n1;
    
    
end;



for k = 1:N
    %e(N+1, k) = e(k, k);
end;