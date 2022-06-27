import numpy as np


def forward(A, B, pi, O):
    """
    Calculates the probability of an observation sequence O given the model(A, B, pi).
    :param A: state transition probabilities (NxN)
    :param B: observation probabilites (NxM)
    :param pi: initial state probabilities (N)
    :param O: sequence of observations(T) where observations are just indices for the columns of B (0-indexed)
        N is the number of states,
        M is the number of possible observations, and
        T is the sequence length.
    :return: The probability of the observation sequence and the calculated alphas in the Trellis diagram with shape
             (N, T) which should be a numpy array.
    """
    N = len(pi)
    T = len(O)
    alphaList = [([0] * T) for _ in range(N)]
    forwardResult = 0
    for t in range(0, T):
        for i in range(0, N):
            for j in range(0, N):
                if (t == 0):
                    alphaList[i][t] = pi[i] * B[i][O[t]]
                    break
                else:
                    alphaList[i][t] += alphaList[j][t - 1] * A[j][i]
            if(t != 0):
                alphaList[i][t] *= B[i][O[t]]
            if(t == T - 1):
                forwardResult += alphaList[i][t]



    return forwardResult,np.asarray(alphaList)


def viterbi(A, B, pi, O):
    """
    Calculates the most likely state sequence given model(A, B, pi) and observation sequence.
    :param A: state transition probabilities (NxN)
    :param B: observation probabilites (NxM)
    :param pi: initial state probabilities(N)
    :param O: sequence of observations(T) where observations are just indices for the columns of B (0-indexed)
        N is the number of states,
        M is the number of possible observations, and
        T is the sequence length.
    :return: The most likely state sequence with shape (T,) and the calculated deltas in the Trellis diagram with shape
             (N, T). They should be numpy arrays.
    """
    N = len(pi)
    T = len(O)
    deltaList = [([0] * T) for _ in range(N)]
    for t in range(0, T):
        for i in range(0, N):
            max = 0
            for j in range(0, N):
                if (t == 0):
                    deltaList[i][t] = pi[i] * B[i][O[t]]
                    break
                else:

                    if(deltaList[j][t - 1] * A[j][i] > max):
                        max = deltaList[j][t - 1] * A[j][i]


            if(t != 0):
                deltaList[i][t] = max * B[i][O[t]]
    deltaList = np.asarray(deltaList)
    viterbiResult = np.argmax(deltaList,axis=0)

    return viterbiResult,deltaList



A = np.asarray([[0.3, 0.7],
                [0.8, 0.2]])

B = np.asarray([[0.5, 0.3, 0.2],
                [0.1, 0.1, 0.8]])

pi = np.asarray([0.6, 0.4])

O = np.asarray([1, 2, 1, 0])

alpha_gt = np.asarray([[0.18, 0.0172, 0.027276, 0.0054306],
                       [0.04, 0.1072, 0.003348, 0.00197628]])
forward_result_gt = 0.00740688

delta_gt = np.asarray([[0.18, 0.0108, 0.024192, 0.0036288],
                       [0.04, 0.1008, 0.002016, 0.00169344]])
viterbi_result_gt = np.asarray([0, 1, 0, 0])

# Testing
forward_result, alpha = forward(A, B, pi, O)
print('Forward result test: {}'.format(abs(forward_result_gt - forward_result) < 10 ** -5))
print('Forward alpha test: {}'.format(np.all(np.abs(alpha - alpha_gt) < 10 ** -5)))

viterbi_result, delta = viterbi(A, B, pi, O)
print('Viterbi result test: {}'.format((viterbi_result_gt == viterbi_result).all()))
print('Viterbi delta test: {}'.format(np.all(np.abs(delta - delta_gt) < 10 ** -5)))
