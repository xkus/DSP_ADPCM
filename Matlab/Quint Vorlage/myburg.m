function [eOut,bOut,e,b,gammaO] = myburg(x,m_max)

mi=1;

n = length(x);
v = zeros(m_max+1,1);
g = zeros(m_max+1,1);
e = zeros(m_max+1,n);
b = zeros(m_max+1,n);        
eOut=zeros(1,n);
bOut=zeros(1,n);

for i=1:n
    e(1,i)=x(i);
    b(1,i)=x(i);
end

v(0+mi) = 1/n*sum(x.*conj(x));
g(0+mi)=-1;

for i=1:m_max    %% i ist Ordnung!
    g(i+mi)=gammaF(e(i-1+mi,:),b(i-1+mi,:),i);
    v(i+mi)=sigma(g(i+mi),v(i-1+mi));
    
    b(i+mi,i+mi:n) = b(i-1+mi,i-1+mi:n-1) - g(i+mi)*e(i-1+mi,i+mi:n);
    e(i+mi,i+mi:n) = e(i-1+mi,i+mi:n) - g(i+mi)*b(i-1+mi,i-1+mi:n-1);

end 

for i=0+mi:m_max-1+mi
    eOut(i)=e(i,i);
end 

eOut(m_max+mi:n)=e(m_max+mi,m_max+mi:n);
bOut(m_max+mi:n)=b(m_max+mi,m_max+mi:n);
gammaO = g;

end


function [new_gamma] = gammaF( fe,be,m ) % fe forward error, be backward error, m - Ordnung

mi = 1;

s1=sum(fe(m+mi:end).*be(m-1+mi:end-1)); 
s2=sum(fe(m+mi:end).*conj(fe(m+mi:end)) + be(m-1+mi:end-1).*conj(be(m-1+mi:end-1)));
new_gamma=2*s1/s2;
end

function [new_sigma]=sigma(gamma, old_sigma)
     new_sigma=(1-gamma*conj(gamma))*power(old_sigma,2);
end







