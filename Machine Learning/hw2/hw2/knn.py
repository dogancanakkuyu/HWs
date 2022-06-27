import numpy as np
import math
import matplotlib.pyplot as plt



def calculate_distances(train_data, test_instance, distance_metric):
    """
    Calculates Manhattan (L1) / Euclidean (L2) distances between test_instance and every train instance.
    :param train_data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data.
    :param test_instance: A (D, ) shaped numpy array.
    :param distance_metric: A string which indicates the distance metric, it can be either 'L1' or 'L2'
    :return: An (N, ) shaped numpy array that contains distances.
    """
    distances = []
    if distance_metric == 'L1':
        for i in range (0, len(train_data)):
            sum = 0
            for k in range (0, len(test_instance)):
                sum += abs(train_data[i][k] - test_instance[k])
            distances.append(sum)
    else:
        for i in range (0, len(train_data)):
            sum = 0
            for k in range(0, len(test_instance)):
                sum += (train_data[i][k] - test_instance[k]) ** 2
            distances.append(math.sqrt(sum))
    return np.array(distances)

def majority_voting(distances, labels, k):
    """
    Applies majority voting. If there are more then one major class, returns the smallest label.
    :param distances: An (N, ) shaped numpy array that contains distances
    :param labels: An (N, ) shaped numpy array that contains labels
    :param k: An integer. The number of nearest neighbor to be selected.
    :return: An integer. The label of the majority class.
    """
    neighboursIndexes = distances.argsort()[:k]
    neighboursLabels = []
    for i in range (0, k):
        neighboursLabels.append(labels[neighboursIndexes[i]])
    majorLabels = []
    maxOccuracy = 0
    for label in neighboursLabels:
        if label not in majorLabels:
            if neighboursLabels.count(label) > maxOccuracy:
                maxOccuracy = neighboursLabels.count(label)
                majorLabels.clear()
                majorLabels.append(label)
            elif neighboursLabels.count(label) == maxOccuracy:
                majorLabels.append(label)
    if len(majorLabels) > 1:
        majorLabels.sort()
        return majorLabels[0]
    else:
        return majorLabels[0]


def knn(train_data, train_labels, test_data, test_labels, k, distance_metric):
    """
    Calculates accuracy of knn on test data using train_data.
    :param train_data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data
    :param train_labels: An (N, ) shaped numpy array that contains labels
    :param test_data: An (M, D) shaped numpy array where M is the number of examples
    and D is the dimension of the data
    :param test_labels: An (M, ) shaped numpy array that contains labels
    :param k: An integer. The number of nearest neighbor to be selected.
    :param distance_metric: A string which indicates the distance metric, it can be either 'L1' or 'L2'
    :return: A float. The calculated accuracy.
    """
    accuratePrediction = 0
    for i in range (0, len(test_data)):
        distances = calculate_distances(train_data,test_data[i],distance_metric)
        predictedLabel = majority_voting(distances,train_labels,k)
        if predictedLabel == test_labels[i]:
            accuratePrediction += 1
    accuracy = accuratePrediction / len(test_data)
    return accuracy


def split_train_and_validation(whole_train_data, whole_train_labels, validation_index, k_fold):
    """
    Splits training dataset into k and returns the validation_indexth one as the
    validation set and others as the training set. You can assume k_fold divides N.
    :param whole_train_data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data
    :param whole_train_labels: An (N, ) shaped numpy array that contains labels
    :param validation_index: An integer. 0 <= validation_index < k_fold. Specifies which fold
    will be assigned as validation set.
    :param k_fold: The number of groups that the whole_train_data will be divided into.
    :return: train_data, train_labels, validation_data, validation_labels
    train_data.shape is (N-N/k_fold, D).
    train_labels.shape is (N-N/k_fold, ).
    validation_data.shape is (N/k_fold, D).
    validation_labels.shape is (N/k_fold, ).
    """
    splittedData = np.array_split(whole_train_data,k_fold)
    splittedLabel = np.array_split(whole_train_labels,k_fold)
    validation_data = splittedData[validation_index]
    validation_labels = splittedLabel[validation_index]

    deletedData = np.delete(splittedData,validation_index,0)
    deletedLabel = np.delete(splittedLabel,validation_index,0)

    train_data = deletedData[0]
    for i in range (1, len(deletedData)):
        train_data = np.concatenate((train_data,deletedData[i]))
    train_labels = deletedLabel[0]
    for k in range (1, len(deletedLabel)):
        train_labels = np.concatenate((train_labels,deletedLabel[k]))

    return train_data,train_labels,validation_data,validation_labels

def cross_validation(whole_train_data, whole_train_labels, k_fold, k, distance_metric):
    """
    Applies k_fold cross-validation and averages the calculated accuracies.
    :param whole_train_data: An (N, D) shaped numpy array where N is the number of examples
    and D is the dimension of the data
    :param whole_train_labels: An (N, ) shaped numpy array that contains labels
    :param k_fold: An integer.
    :param k: An integer. The number of nearest neighbor to be selected.
    :param distance_metric: A string which indicates the distance metric, it can be either 'L1' or 'L2'
    :return: A float. Average accuracy calculated.
    """
    sumOfAccuracies = 0
    for validationIndex in range (0, k_fold):
        train_data, train_labels, test_data, test_labels = split_train_and_validation(whole_train_data,whole_train_labels,validationIndex,k_fold)
        sumOfAccuracies += knn(train_data,train_labels,test_data,test_labels,k,distance_metric)
    averageAccuracy = sumOfAccuracies / k_fold
    return averageAccuracy

