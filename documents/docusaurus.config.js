module.exports = {
  title: 'XSTAR 文档',
  url: 'https://xstar.org',
  baseUrl: '/',
  onBrokenLinks: 'warn',
  markdown: {
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },
  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          path: 'docs',
          routeBasePath: '/',
          sidebarPath: require.resolve('./sidebars.js'),
        },
        blog: false,
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],
  plugins: [
    [
      require.resolve('@easyops-cn/docusaurus-search-local'),
      {
        hashed: true,
        language: ['zh', 'en'],
        indexBlog: false,
        indexDocs: true,
        explicitSearchResultPath: true,
        highlightSearchTermsOnTargetPage: true,
      },
    ],
  ],
  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      metadata: [{ name: 'keywords', content: 'xstar, embedded, framework, rtos' }],
      colorMode: {
        defaultMode: 'light',
        disableSwitch: false,
        respectPrefersColorScheme: true,
      },
      navbar: {
        title: 'XSTAR',
        items: [
          {
            type: 'docSidebar',
            sidebarId: 'guide',
            position: 'left',
            label: '入门指南',
          },
          {
            type: 'docSidebar',
            sidebarId: 'subsystems',
            position: 'left',
            label: '子系统',
          },
          {
            type: 'docSidebar',
            sidebarId: 'driver',
            position: 'left',
            label: '设备驱动',
          },
          {
            type: 'docSidebar',
            sidebarId: 'command',
            position: 'left',
            label: '内置命令',
          },
          {
            type: 'docSidebar',
            sidebarId: 'libx',
            position: 'left',
            label: '基础库',
          },
          {
            type: 'docSidebar',
            sidebarId: 'package',
            position: 'left',
            label: '扩展包',
          },
          {
            type: 'localeDropdown',
            position: 'right',
          },
        ],
      },
      footer: {
        style: 'dark',
        copyright: `Copyright © ${new Date().getFullYear()} XSTAR. Built with Docusaurus.`,
      },
      prism: {
        theme: require('prism-react-renderer').themes.github,
        darkTheme: require('prism-react-renderer').themes.dracula,
        additionalLanguages: ['bash', 'c', 'cpp', 'json', 'makefile'],
      },
      tableOfContents: {
        minHeadingLevel: 2,
        maxHeadingLevel: 4,
      },
    }),
  i18n: {
    defaultLocale: 'zh',
    locales: ['zh', 'en'],
  },
};
