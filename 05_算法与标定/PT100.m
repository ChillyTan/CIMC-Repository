A = 3.9083e-3;
B = -5.775e-7;
C = -4.183e-12;     % 如果你的公式中不需要 C，可以改成 0
R0 = 100;           % 0℃ 时的电阻，PT100 为 100Ω

% 示例1：根据温度计算电阻
T = 0;             % 温度，单位 ℃
R = temp_to_res(T, R0, A, B, C);
fprintf('温度 %.4f ℃ 对应的电阻为 %.6f Ω\n', T, R);

% 110.66
% 112.6
% 122.9
% 130.9
% 138.85
% 150.2
% 160.15
arr_R = [
80.49
82.42
100.17
107.72
112.60 
115.28
122.86
130.81
138.74
150.05
153.90 
];

for i = 1:length(arr_R)
    % 示例2：根据电阻计算温度
    R_meas = arr_R(i);  % 测得电阻，单位 Ω
    T_calc = res_to_temp(R_meas, R0, A, B, C);
    T_mode = (R_meas - 100) / 0.385055;
    fprintf('电阻 %.6f Ω 对应的温度为 %.6f ℃ 简单线性温度为 %.6f ℃\n', R_meas, T_calc, T_mode);
end

% % 示例2：根据电阻计算温度
% R_meas = 138.85;  % 测得电阻，单位 Ω
% T_calc = res_to_temp(R_meas, R0, A, B, C);
% T_mode = (R_meas - 100) / 0.385055;
% fprintf('电阻 %.6f Ω 对应的温度为 %.6f ℃ 简单线性温度为 %.6f ℃\n', R_meas, T_calc, T_mode);


% ================= 函数区 =================

function R = temp_to_res(T, R0, A, B, C)
    % 根据温度计算电阻
    % 支持标量或数组输入

    R = zeros(size(T));

    idx_neg = T < 0;
    idx_pos = T >= 0;   

    % T < 0
    R(idx_neg) = R0 .* (1 + A .* T(idx_neg) + B .* T(idx_neg).^2);

    % T >= 0
    R(idx_pos) = R0 .* (1 + A .* T(idx_pos) + B .* T(idx_pos).^2 ...
        + C .* (T(idx_pos) - 100) .* T(idx_pos).^3);
end


function T = res_to_temp(R, R0, A, B, C)
    % 根据电阻反算温度
    % 使用 fzero 数值求解
    % 支持标量或数组输入

    T = zeros(size(R));

    for k = 1:numel(R)
        Rk = R(k);

        % 根据 PT100 特性，R < R0 一般对应 T < 0
        if Rk < R0
            T_range = [-200, 0];
        else
            T_range = [0, 850];
        end

        fun = @(T) temp_to_res(T, R0, A, B, C) - Rk;

        T(k) = fzero(fun, T_range);
    end
end