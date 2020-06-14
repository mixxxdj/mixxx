module.exports = {
  presets: [
    ['@babel/preset-env', { 'modules': false }]
  ],
  plugins: [
    'transform-es3-member-expression-literals',
    'transform-es3-property-literals',
    '@babel/plugin-proposal-class-properties',
    '@babel/plugin-transform-flow-strip-types',
    ['@babel/plugin-transform-runtime', { useESModules: true }]
  ]
}
