%You must have the data from "data_forKalman_Filter_HWProblem.m" loaded in Matlab's
%memory before executing this M-file.
% type at the Matlab prompt >> data_forKalman_Filter_HWPrblem
% then type edit KalmanFilterHWProblem.m if you have not done so already

x_pred = [0;0;0]; %[x,y,theta]
z = [0;0;0]; %[Optitrack x, Optitrack y, Optitrack Theta
ytilde = [0;0;0];
K = eye(3);

ProcUncert = 0.15;
Q = [ProcUncert 0 ProcUncert/10;
	0 ProcUncert ProcUncert/10;
	ProcUncert/10 ProcUncert/10 ProcUncert];
MeasUncert = 3;
R = [MeasUncert 0 MeasUncert/10;
	0 MeasUncert MeasUncert/10;
	MeasUncert/10 MeasUncert/10 MeasUncert];
S = eye(3);
P_pred = eye(3);

x = zeros(200000,1);
y = zeros(200000,1);
theta = zeros(200000,1);
opti_x = zeros(17593,1);  % in this data set there happens to be 17593 Optitrack data points
opti_y = zeros(17593,1);

numoptitrackpoints = 0;
x(1) = -3.814735; % The x,y,theta location where I happened to start the robot for this run
y(1) = 6.565212;
theta(1) = -1.588956;
x_pred(1,1) = x(1);
x_pred(2,1) = y(1);
x_pred(3,1) = theta(1);

F = eye(3);
H = eye(3);
P = eye(3);

U = data(:, 2:3);
Z = data(:, 4:6);

for i=1:199999  %Loop through all the data.  Each data point is 0.001s.  
    %Prediction steps that happen every loop.  
    
    %Helping out here because B*u is a bit confusing because is uses a previous step back theta
    uk = [data(i,2);data(i,3)]; %[Average wheel velocity, gyro reading]
    B = [cos(theta(i))*0.001 0;sin(theta(i))*0.001 0;0 0.001];
    u = U(i,:)';
    
    %Calcuate x_pred        (xhatK+1|K) = F*(xhatK|K) + B*uK  
    x_hat = F*x_pred + B*u;
    y_hat = H*x_hat;
    %Calculate P_pred  (PK+1|K) = F*(PK|K)*F' + Q 
    P_hat = F*P*F'+Q;

    
    
    % Correction Steps
    % In the data set I collected when there was no new Optitrack data I
    % assigned the Optitrack data equal to 0.0.  So if the data point is
    % not equal to zero it is a new Optitrack point
    if data(i,4) ~= 0.0  
        numoptitrackpoints = numoptitrackpoints + 1;
        %log for plotting
        opti_x(numoptitrackpoints) = data(i,4);
        opti_y(numoptitrackpoints) = data(i,5);
        
        %z is current Optitrack measurement
        z(1,1) = data(i,4);
        z(2,1) = data(i,5);
        if cos(theta(i)) < -0.99  %Optitrack has difficulty with this angle
            z(3,1) = theta(i);
        else
            z(3,1) = data(i,6);
        end
        % ytilde = zK+1 - H*(xhatK+1|K)

        % S = H*(PK+1|K)*H' + R

        % K = (PK+1|K)*H'*S^-1

        % (xhatK+1|K+1) = (xhatK+1|K) + K*ytilde

        % (PK+1|K+1) = (I - K*H)*(PK+1|K)
        
        error = z - H*(x_pred);
        S = H*(P_hat)*H' + R;
        K = (P_hat)*H'*S^-1;
        x_pred = x_hat+K*error;
        P = (eye(3) - K*H)*P_hat;

    end
    
    % Log for plotting
    x(i+1) = x_pred(1,1);
    y(i+1) = x_pred(2,1);
    theta(i+1) = x_pred(3,1);
    
end    

figure(1)
plot(opti_x,opti_y,'.')
title('Plot of all the Optitrack x,y points');
figure(2)
plot(x,y)
title('Plot of Fused Dead Reckoned Data and Optitrack Data');
figure(3)
plot(opti_x(5200:8690),opti_y(5200:8690),'.',x(60000:100000),y(60000:100000))
title('Plot of Subset of Optitrack points and Fused Data, Note the point with large error');