import math

import numpy as np
import matplotlib.pyplot as plt




def find_min_indexes(arr):
    minD = 999999
    ind1 = 0; ind2 = 0
    for i in range (len(arr)):
        for k in range (len(arr)):
            if arr[i][k] != 0 and arr[i][k] < minD:
                minD = arr[i][k]
                ind1 = i; ind2 = k
    return ind1,ind2
def single_linkage(c1, c2):
    """
    Given clusters c1 and c2, calculates the single linkage criterion.
    :param c1: An (N, D) shaped numpy array containing the data points in cluster c1.
    :param c2: An (M, D) shaped numpy array containing the data points in cluster c2.
    :return: A float. The result of the calculation.
    """
    if (len(c1.shape) == 2 and len(c2.shape) == 1):
        min = 9999999
        for i in c1:
            d = np.linalg.norm(i - c2)
            if d < min:
                min = d
        return min
    elif (len(c1.shape) == 1 and len(c2.shape) == 2):
        min = 9999999
        for i in c2:
            d = np.linalg.norm(i - c1)
            if d < min:
                min = d
        return min
    else:
        min = 999999
        for i in range (0, len(c1)):
            for k in range (0, len(c2)):
                if (np.array_equal(c1[i],c2[k])) == False:
                    d = np.linalg.norm(c1[i] - c2[k])
                    if d < min:
                        min = d
        return min

def complete_linkage(c1, c2):
    """
    Given clusters c1 and c2, calculates the complete linkage criterion.
    :param c1: An (N, D) shaped numpy array containing the data points in cluster c1.
    :param c2: An (M, D) shaped numpy array containing the data points in cluster c2.
    :return: A float. The result of the calculation.
    """
    if (len(c1.shape) == 2 and len(c2.shape) == 1):
        max = 0
        for i in c1:
            d = np.linalg.norm(i - c2)
            if d > max:
                max = d
        return max
    elif (len(c1.shape) == 1 and len(c2.shape) == 2):
        max = 0
        for i in c2:
            d = np.linalg.norm(i - c1)
            if d < max:
                max = d
        return max
    else:
        max = 0
        for i in range(0, len(c1)):
            for k in range(0, len(c2)):
                if (np.array_equal(c1[i], c2[k])) == False:
                    d = np.linalg.norm(c1[i] - c2[k])
                    if d > max:
                        max = d
        return max


def average_linkage(c1, c2):
    """
    Given clusters c1 and c2, calculates the average linkage criterion.
    :param c1: An (N, D) shaped numpy array containing the data points in cluster c1.
    :param c2: An (M, D) shaped numpy array containing the data points in cluster c2.
    :return: A float. The result of the calculation.
    """
    if (len(c1.shape) == 2 and len(c2.shape) == 1):
        sum = 0
        for i in c1:
            sum += np.linalg.norm(i - c2)
        return sum / (len(c1))
    elif (len(c1.shape) == 1 and len(c2.shape) == 2):
        sum = 0
        for i in c2:
            sum += np.linalg.norm(i - c1)
        return sum / (len(c2))
    else:
        sum = 0
        for i in range(0, len(c1)):
            for k in range(0, len(c2)):
                sum += np.linalg.norm(c1[i] - c2[k])
        return sum / (len(c1) * len(c2))

def centroid_linkage(c1, c2):
    """
    Given clusters c1 and c2, calculates the centroid linkage criterion.
    :param c1: An (N, D) shaped numpy array containing the data points in cluster c1.
    :param c2: An (M, D) shaped numpy array containing the data points in cluster c2.
    :return: A float. The result of the calculation.
    """
    if (len(c1.shape) == 2 and len(c2.shape) == 2):
        center1 = np.zeros(c1.shape[1])
        center2 = np.zeros(c2.shape[1])
        for i in range (0,len(c1)):
            for k in range (0, len(center1)):
                center1[k] += c1[i][k]
        for i in range (0,len(c2)):
            for k in range (0,len(center2)):
                center2[k] += c2[i][k]
        center1 /= len(c1)
        center2 /= len(c2)
        dist = np.linalg.norm(center1 - center2)
        return dist
    elif (len(c1.shape) == 2 and len(c2.shape) == 1):
        center1 = np.zeros(c1.shape[1])
        for i in range (0,len(c1)):
            for k in range (0, len(center1)):
                center1[k] += c1[i][k]
        center1 /= len(c1)
        dist = np.linalg.norm(center1 - c2)
        return dist
    else:
        center2 = np.zeros(c2.shape[1])
        for i in range(0, len(c2)):
            for k in range(0, len(center2)):
                center2[k] += c2[i][k]
        center2 /= len(c1)
        dist = np.linalg.norm(center2 - c1)
        return dist


def hac(data, criterion, stop_length):
    """
    Applies hierarchical agglomerative clustering algorithm with the given criterion on the data
    until the number of clusters reaches the stop_length.
    :param data: An (N, D) shaped numpy array containing all of the data points.
    :param criterion: A function. It can be single_linkage, complete_linkage, average_linkage, or
    centroid_linkage
    :param stop_length: An integer. The length at which the algorithm stops.
    :return: A list of numpy arrays with length stop_length. Each item in the list is a cluster
    and a (Ni, D) sized numpy array.
    """

    distanceArr = np.array([])
    clusterArr = []
    for i in range (len(data)):
        dummyL = []
        clusterArr.append(i)
        for k in range (len(data)):
            d = np.linalg.norm(data[i] - data[k])
            dummyL.append(d)
        dummyL = np.array(dummyL)
        distanceArr = np.append(distanceArr,dummyL,axis=0)
    distanceArr = np.reshape(distanceArr,(len(data),len(data)))


    while (len(distanceArr) > stop_length):
        ind1, ind2 = find_min_indexes(distanceArr)
        if (type(clusterArr[ind1]) == int and type(clusterArr[ind2]) == int):
            dummy = [clusterArr[ind1], clusterArr[ind2]]
        elif (type(clusterArr[ind1]) == list and type(clusterArr[ind2]) == int):
            x = clusterArr[ind1]
            y = clusterArr[ind2]
            x.append(y)
            dummy = x
        elif (type(clusterArr[ind1]) == int and type(clusterArr[ind2]) == list):
            x = clusterArr[ind2]
            y = clusterArr[ind1]
            x.append(y)
            dummy = x

        else:
            dummy = clusterArr[ind1] + clusterArr[ind2]

        if ind1 < ind2:
            del clusterArr[ind1]
            del clusterArr[ind2 - 1]
        else:
            del clusterArr[ind2]
            del clusterArr[ind1 - 1]
        clusterArr.append(dummy)

        tempL = np.vstack((data[clusterArr[-1][0]],data[clusterArr[-1][1]]))
        for i in range (2, len(clusterArr[-1])):
            tempL = np.vstack((tempL,data[clusterArr[-1][i]]))
        distMatrix = []

        for i in range (0,len(clusterArr) - 1):
            if (type(clusterArr[i]) == list):
                tempL2 = np.vstack((data[clusterArr[i][0]], data[clusterArr[i][1]]))
                for k in range(2, len(clusterArr[i])):
                    tempL2 = np.vstack((tempL2, data[clusterArr[i][k]]))
                d = criterion(tempL,tempL2)
                distMatrix.append(d)
            else:
                distMatrix.append(criterion(tempL,data[clusterArr[i]]))
        distMatrix.append(0)
        distanceArr = np.delete(distanceArr, [ind1, ind2], 0)
        tempDistArr = np.array([])
        for i in range (0, len(distanceArr)):
            tempElem = np.delete(distanceArr[i],[ind1,ind2],0)
            tempElem = np.insert(tempElem,len(tempElem),distMatrix[i])
            tempDistArr = np.append(tempDistArr,tempElem,axis=0)
        distMatrix = np.array(distMatrix)
        tempDistArr = np.append(tempDistArr,distMatrix,0)
        distanceArr = np.reshape(tempDistArr,(len(distMatrix),len(distMatrix)))

    resultArr = []
    for i in range (0,len(clusterArr)):
        temp = []
        if type(clusterArr[i]) != list:
            temp.append(data[clusterArr[i]])
        else:
            for k in range (0,len(clusterArr[i])):
                temp.append(data[clusterArr[i][k]])
        temp = np.array(temp)
        resultArr.append(temp)
    return np.array(resultArr,dtype=object)






















