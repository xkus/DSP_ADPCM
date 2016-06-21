function [ x ] = decoder2(input,g)

mi = 1;

K=length(input);
order=length(g)-1;
x=zeros(order+1,1);
e=zeros(order+1,K);
b=zeros(order+1,K);

e(0+mi,0+mi)=input(0+mi);
b(0+mi,0+mi)=e(0+mi,0+mi);

for k = 1:K-1%timpul
    e(min(k,order)+mi,k+mi)=input(k+mi);
    for n=min(k,order):-1:1% for e, Ordnung
        e(n-1+mi,k+mi) = getVal(e,n+mi,k+mi)+g(n+mi)*getVal(b,n-1+mi,k-1+mi);
        b(n+mi,k+mi) = getVal(b,n-1+mi,k-1+mi)-g(n+mi)*getVal(e,n-1+mi,k+mi);
    end%for b, Ordnung
    b(0+mi,k+mi)=e(0+mi,k+mi);
end % for Zeit

x = e(0+mi,:);

end


function v = getVal(arr,zeile,spalte)
    if zeile<1 || spalte<1
        v = 0;
    else
        v=arr(zeile,spalte);
    end
end