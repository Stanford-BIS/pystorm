pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                sh 'docker build --file docker/Dockerfile_JENKINS_CI -t ${BRANCH_NAME}-${BUILD_NUMBER} .'
            }
        }
        stage('Test') {
            steps {
                sh 'docker run --rm -i ${BRANCH_NAME}-${BUILD_NUMBER} test ARGS="-V"'
            }
        }
    }
}
