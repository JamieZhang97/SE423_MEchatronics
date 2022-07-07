g = 9.81;
m = 0.03;
R = 0.04;
J_w = (m*R^2)/2;
M = 0.6;
L = 0.144/2;
J_psi = (M*L^2)/3;
J_m = 1e-5;
R_m = 6.69;
K_b = 0.468;
K_t = 0.317;
n = 1;
f_m = 0.0022;
alpha = (n*K_t)/R_m;
beta = ((n*K_t*K_b)/R_m) + f_m;

D = [(M*L^2 + J_psi + 2*n^2*J_m) (M*L*R - 2*n^2*J_m);
    (M*L*R - 2*n^2*J_m) ((2*m+M)*R^2 + 2*J_w + 2*n^2*J_m)];
b = [M*g*L -2*beta 0 2*beta;
    0 2*beta 0 -2*beta];
h = [-2*alpha;
    2*alpha];
Dib = inv(D)*b;
Dih = inv(D)*h;

A = [0 1 0 0;
    Dib(1,:);
    0 0 0 1;
    Dib(2,:)]
B = [0;
    Dih(1);
    0;
    Dih(2)]
A3 = [A(1,1) A(1,2) A(1,4);
    A(2,1) A(2,2) A(2,4);
    A(4,1) A(4,2) A(4,4)]
B3 = [B(1);
    B(2);
    B(4)]
% K3 = [-8.7484  -13.7866   -6.6363]; %Bad set of gains to start
eig(A3)
K3 = place(A3, B3, [-241.1937  -10  -6.5199]);
K = [-151.7589  -14.8192  -61.2183   -7.1023];  %Bad set of gains to start
