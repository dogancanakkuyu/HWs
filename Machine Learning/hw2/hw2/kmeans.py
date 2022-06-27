import numpy as np
import math
import random



def centroidInıtialization(data,k):

    minVal = data.min().min()
    maxVal = data.max().max()
    dataLen = data.shape[1]
    centroids = []

    for center in range(k):
        center = np.random.uniform(minVal, maxVal, dataLen)
        centroids.append(center)

    return np.array(centroids)

def assign_clusters(data, cluster_centers):
    """
    Assigns every data point to its closest (in terms of Euclidean distance) cluster center.
    :param data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data
    :param cluster_centers: A (K, D) shaped numpy array where K is the number of clusters
    and D is the dimension of the data
    :return: An (N, ) shaped numpy array. At its index i, the index of the closest center
    resides to the ith data point.
    """
    assignment = []
    for i in range (0, len(data)):
        distance = 9999999999
        assignedIndex = 0
        for k in range (0, len(cluster_centers)):
            d = 0
            for j in range (0, data.shape[1]):
                d += (data[i][j] - cluster_centers[k][j]) ** 2
            d = math.sqrt(d)
            if d < distance:
                distance = d
                assignedIndex = k
        assignment.append(assignedIndex)

    return np.array(assignment)

def calculate_cluster_centers(data, assignments, cluster_centers, k):
    """
    Calculates cluster_centers such that their squared Euclidean distance to the data assigned to
    them will be lowest.
    If none of the data points belongs to some cluster center, then assign it to its previous value.
    :param data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data
    :param assignments: An (N, ) shaped numpy array with integers inside. They represent the cluster index
    every data assigned to.
    :param cluster_centers: A (K, D) shaped numpy array where K is the number of clusters
    and D is the dimension of the data
    :param k: Number of clusters
    :return: A (K, D) shaped numpy array that contains the newly calculated cluster centers.
    """
    sumOfDataPointsInCluster = []
    noOfElementsInCluster = []
    for i in range(0, k):
        sumOfDataPointsInCluster.append([])
        for k in range(0, data.shape[1]):
            sumOfDataPointsInCluster[i].append(0)
        noOfElementsInCluster.append(0)
    for i in range(0, len(data)):
        noOfElementsInCluster[assignments[i]] += 1
        for k in range(0, data.shape[1]):
            sumOfDataPointsInCluster[assignments[i]][k] += data[i][k]
    for i in range(0, len(sumOfDataPointsInCluster)):
        for k in range(0, len(sumOfDataPointsInCluster[i])):
            if noOfElementsInCluster[i] == 0:
                sumOfDataPointsInCluster[i][k] = cluster_centers[i][k]
            else:
                sumOfDataPointsInCluster[i][k] = sumOfDataPointsInCluster[i][k] / noOfElementsInCluster[i]
    return np.array(sumOfDataPointsInCluster)


def kmeans(data, initial_cluster_centers):
    """
    Applies k-means algorithm.
    :param data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data
    :param initial_cluster_centers: A (K, D) shaped numpy array where K is the number of clusters
    and D is the dimension of the data
    :return: cluster_centers, objective_function
    cluster_center.shape is (K, D).
    objective function is a float. It is calculated by summing the squared euclidean distance between
    data points and their cluster centers.
    """

    while (True):
        assignment = assign_clusters(data, initial_cluster_centers)
        objectiveFunc = 0
        for i in range(0, len(data)):
            objectiveFunc += np.linalg.norm(data[i] - initial_cluster_centers[assignment[i]]) ** 2
        new_center = calculate_cluster_centers(data, assignment, initial_cluster_centers, len(initial_cluster_centers))
        new_assignment = assign_clusters(data,new_center)
        new_objectiveFunc = 0
        for i in range(0, len(data)):
            new_objectiveFunc += np.linalg.norm(data[i] - new_center[new_assignment[i]]) ** 2
        if new_objectiveFunc == objectiveFunc:
            cluster_centers = new_center
            objective_function = new_objectiveFunc
            return cluster_centers, objective_function / 2
        else:
            initial_cluster_centers = new_center


