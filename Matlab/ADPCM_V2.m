% ADPCM Encoder
%clear all
% read wave file
%[FileName,PathName] = uigetfile('*','Select sound file');
%[a, Fs] = audioread([PathName FileName], 'native');

% 50ms sample of wave file
%anz = 500;
%pos = Fs * 50;  % Set cut offset (Cut of top of song)
%x = a(pos:pos+anz-1, 1);    % left channel


%% Encode vector x


%GET SIZE FROM INPUT VECTORS
N = length(x) - 1;
m = 6;
%INITIALIZE Ak
Ak = zeros(1,m + 1);
Ak(1) = 1;
%INITIALIZE f and b
f = x;
b = x;
%INITIALIZE Dk
Dk = 0;
for j = 1:1:N+1 %j = 0; j <= N; j++
Dk = Dk + 2.0 * f(j)*f(j);
end;

Dk = Dk -f(0+1)* f(0+1) + b(N+1) * b(N+1);
%BURG RECURSION
for k = 1:1:m+1 %k = 0; k < m; k++ )
    %COMPUTE MU
    mu = 0.0;
    for n = 1:1:N-k-1+1 %n = 0; n <= N - k - 1; n++ )
        mu = mu + f( n + k + 1 ) * b(n);
    end;

    mu = mu * ( -2.0 / Dk);
    %UPDATE Ak
    for n = 1:1:(k+2)/2  %n = 0; n <= ( k + 1 ) / 2; n++ )

        t1 = Ak(n) + mu * Ak(k + 1 - n);
        t2 = Ak(k + 1 - n ) + mu * Ak( n);
        Ak(n) = t1;
        Ak(k + 1 - n ) = t2;

    end;

    %UPDATE f and b
    for n = 1:1:N-k %n = 0; n <= N - k - 1; n++ )

        t1 = f( n + k + 1 ) + mu * b( n );
        t2 = b( n ) + mu * f( n + k + 1 );
        f( n + k + 1 ) = t1;
        b( n ) = t2;

    end;

    %UPDATE Dk
    Dk = ( 1.0 - mu * mu ) * Dk - f( k + 1 + 1 ) * f( k + 1 +1 ) - b( N - k - 1 +1) * b( N - k - 1 +1);

end;

input('\n\nHier HALT!\n')

%% Decode with coefficients e
%clearvars -except y e N Fs x

e = e(7,:);

ef = zeros(1,7);
bf = zeros(1,7);

% ADPCM decoder

for k = 1:length(e)
    ef(7) = e(k);  % Neuer Wert
    
for i = N:-1:1
   ef(i) = ef(i+1) + bf(i)*y(i);
   bf(i+1) = ef(i)*(-y(i))+bf(i);
end;
bf(1) = ef(1);
x1(k) = ef(1);
end;

e_dsp=e.*(127/500);
sound(x1,Fs);

