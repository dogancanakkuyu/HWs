import numpy as np

def vocabulary(data):
    """
    Creates the vocabulary from the data.
    :param data: List of lists, every list inside it contains words in that sentence.
                 len(data) is the number of examples in the data.
    :return: Set of words in the data
    """
    words = set()
    for i in range(len(data)):
        for k in range(len(data[i])):
            words.add(data[i][k])
    return words




def estimate_pi(train_labels):
    """
    Estimates the probability of every class label that occurs in train_labels.
    :param train_labels: List of class names. len(train_labels) is the number of examples in the training data.
    :return: pi. pi is a dictionary. Its keys are class names and values are their probabilities.
    """
    pi = {}
    for label in train_labels:
        pi[label] = 0
    for label in train_labels:
        pi[label] += 1
    for keys in pi.keys():
        pi[keys] /= len(train_labels)
    return pi

def estimate_theta(train_data, train_labels, vocab):
    """
    Estimates the probability of a specific word given class label using additive smoothing with smoothing constant 1.
    :param train_data: List of lists, every list inside it contains words in that sentence.
                       len(train_data) is the number of examples in the training data.
    :param train_labels: List of class names. len(train_labels) is the number of examples in the training data.
    :param vocab: Set of words in the training set.
    :return: theta. theta is a dictionary of dictionaries. At the first level, the keys are the class names. At the
             second level, the keys are all the words in vocab and the values are their estimated probabilities given
             the first level class name.
    """
    theta = {}
    wordCountsForClasses = {} #holds total number of words for each labels
    tlabels = list(set(train_labels))
    for labels in tlabels:
        wordCountsForClasses[labels] = 0
        theta[labels] = {}
        for v in vocab:
            theta[labels][v] = 1


    for i in range(len(train_data)):
        label = train_labels[i]
        wordCountsForClasses[label] += len(train_data[i])

    d = len(vocab)
    for i in range (len(train_data)):
        label = train_labels[i]
        for v in set(train_data[i]):
            theta[label][v] += train_data[i].count(v)
    for keys in theta.keys():
        for v in vocab:
            theta[keys][v] /= (d + wordCountsForClasses[keys])
    return theta





def test(theta, pi, vocab, test_data):
    """
    Calculates the scores of a test data given a class for each class. Skips the words that are not occurring in the
    vocabulary.
    :param theta: A dictionary of dictionaries. At the first level, the keys are the class names. At the second level,
                  the keys are all of the words in vocab and the values are their estimated probabilities.
    :param pi: A dictionary. Its keys are class names and values are their probabilities.
    :param vocab: Set of words in the training set.
    :param test_data: List of lists, every list inside it contains words in that sentence.
                      len(test_data) is the number of examples in the test data.
    :return: scores, list of lists. len(scores) is the number of examples in the test set. Every inner list contains
             tuples where the first element is the score and the second element is the class name.
    """
    scores = []
    for td in test_data:
        temp = []
        for classes in pi.keys():

            score = np.log(pi[classes])

            for v in set(td):
                if v in vocab:
                    score += td.count(v) * np.log(theta[classes][v])
            tupLe = (score,classes)
            temp.append(tupLe)
        scores.append(temp)
    return scores



def readFile(filename): #puts every line in a list
    linesList = []
    with open(filename,'r') as file:
        for line in file:
            line = line.rstrip("\n") #removing end line characters
            linesList.append(line)
    return linesList
def removeSpaces(linesList): #removes spaces between words in a sentence
    f = []
    for element in linesList:
        l = element.split(' ')
        f.append(l)
    return f

def computeAccuracy(scores,testLabels): #computes test accuracy in percentage
    predictions = []
    totalClassNo = len(scores[0])
    for scoreTuples in scores:
        score = scoreTuples[0][0]
        predictedClass = scoreTuples[0][1]
        for i in range(1,totalClassNo):
            if(scoreTuples[i][0] > score):
                score = scoreTuples[i][0]
                predictedClass = scoreTuples[i][1]
        predictions.append(predictedClass)
    accuracy = 0
    for i in range(len(predictions)):
        if(predictions[i] == testLabels[i]):
            accuracy += 1
    return (accuracy / len(predictions)) * 100


trainData = readFile('nb_data/train_set.txt')
trainData = removeSpaces(trainData)
testData = readFile('nb_data/test_set.txt')
testData = removeSpaces(testData)
trainLabels = readFile('nb_data/train_labels.txt')
testLabels = readFile('nb_data/test_labels.txt')
vocab = vocabulary(trainData)
pi = estimate_pi(trainLabels)
theta = estimate_theta(trainData,trainLabels,vocab)
scores = test(theta,pi,vocab,testData)
accuracyPercentage = computeAccuracy(scores,testLabels)
print("Test accuracy: %",accuracyPercentage)







