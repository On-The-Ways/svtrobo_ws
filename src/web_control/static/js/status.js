/**
 * SVTROBO Web Control - Status Monitor Module
 * Battery status (placeholder) + Topic list
 */

const StatusMonitor = {
    ros: null,

    init(ros) {
        this.ros = ros;
        this.refreshTopics();
        // Refresh topic list every 5s
        setInterval(() => this.refreshTopics(), 5000);
    },

    async refreshTopics() {
        if (!this.ros || !this.ros.isConnected) return;

        try {
            const topicsClient = new ROSLIB.Service({
                ros: this.ros,
                name: '/rosapi/topics',
                serviceType: 'rosapi/Topics',
            });

            const request = new ROSLIB.ServiceRequest({});
            topicsClient.callService(request, (result) => {
                this.updateTopicList(result.topics);
            });
        } catch (e) {
            console.error('Failed to fetch topics:', e);
        }
    },

    updateTopicList(topics) {
        const container = document.getElementById('topic-list');
        if (!container) return;

        if (!topics || topics.length === 0) {
            container.innerHTML = '<div class="topic-empty">暂无订阅话题</div>';
            return;
        }

        const sorted = topics.sort();
        container.innerHTML = sorted.map(t => `<div class="topic-item">${t}</div>`).join('');
        document.getElementById('topic-count').textContent = sorted.length;
    },
};
